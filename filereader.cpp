#include "filereader.h"

#include <QUrl>
#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QPointer>

#include <QDebug>

struct JsonLoadResult
{
    bool ok = false;
    QString error;

    QStandardItemModel* model = nullptr;

    qint64 openMs = 0;
    qint64 readMs = 0;
    qint64 parseMs = 0;
    qint64 buildMs = 0;
    qint64 totalMs = 0;
};

FileReader::FileReader(QObject *parent)
    : QObject{parent}
{}

void FileReader::setLoading(bool v)
{
    if (m_loading == v) return;
    m_loading = v;
    emit loadingChanged();
}

void FileReader::setStatusText(const QString &t)
{
    if (m_statusText == t) return;
    m_statusText = t;
    emit statusTextChanged();
}

void FileReader::readJsonFile(const QUrl &url)
{
    if (!url.isLocalFile())
        return;

    setLoading(true);
    setStatusText("Loading (sync) ...");

    QElapsedTimer timer;
    timer.start();

    QFile f(url.toLocalFile());
    if (!f.open(QIODevice::ReadOnly))
    {
        setStatusText("Failed to open file");
        setLoading(false);
        return;
    }

    qint64 openMs = timer.elapsed();

    QByteArray data = f.readAll();

    qint64 readMs = timer.elapsed();

    QJsonParseError e;
    QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if (e.error != QJsonParseError::NoError)
    {
        setStatusText("JSON parse error");
        setLoading(false);
        return;
    }

    qint64 parseMs = timer.elapsed();

    //analyzeJson(doc);

    QStandardItemModel* newModel = buildJsonTreeModel(doc, this, true);

    qint64 buildMs = timer.elapsed();

    qDebug() << "open(ms):" << openMs
             << "read(ms):" << (readMs - openMs)
             << "parse(ms):" << (parseMs - readMs)
             << "build(ms):" << (buildMs - parseMs)
             << "total(ms):" << buildMs;

    setStatusText(QString("Done in %1 ms").arg(timer.elapsed()));
    setLoading(false);

    if (m_jsonModel)
    {
        m_jsonModel->deleteLater();
    }

    m_jsonModel = newModel;

    emit jsonModelChanged();

    return;
}

void FileReader::readJsonFileAsync(const QUrl &url)
{
    if (!url.isLocalFile())
        return;

    if (m_loadInProgress) {
        setStatusText("Load already in progress...");
        return;
    }
    m_loadInProgress = true;

    setLoading(true);
    setStatusText("Loading (async): starting...");

    const QString path = url.toLocalFile();

    // If FileReader gets deleted while the task is running, avoid using a dangling pointer.
    QPointer<FileReader> self(this);

    auto *watcher = new QFutureWatcher<JsonLoadResult>(this);

    connect(watcher, &QFutureWatcher<JsonLoadResult>::finished, this, [this, watcher, self]() {
        const JsonLoadResult r = watcher->result();
        watcher->deleteLater();

        if (!self) {
            // FileReader was deleted; avoid touching members.
            if (r.model) r.model->deleteLater();
            return;
        }

        if (!r.ok) {
            setStatusText("Async load failed: " + r.error);
            setLoading(false);
            m_loadInProgress = false;
            return;
        }

        setStatusText(QString("Async done: open %1ms, read %2ms, parse %3ms, build %4ms, total %5ms")
                          .arg(r.openMs).arg(r.readMs).arg(r.parseMs).arg(r.buildMs).arg(r.totalMs));

        // Swap models on UI thread
        if (m_jsonModel) {
            m_jsonModel->deleteLater();
        }

        m_jsonModel = r.model;
        m_jsonModel->setParent(this); // adopt ownership in UI thread

        emit jsonModelChanged();

        setLoading(false);
        m_loadInProgress = false;
    });

    watcher->setFuture(QtConcurrent::run([path, sortKeys = true, self]() -> JsonLoadResult {
        JsonLoadResult out;

        // If FileReader disappears, we still can finish; we just must not dereference self.
        QElapsedTimer timer;
        timer.start();

        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            out.error = "Failed to open file";
            return out;
        }
        out.openMs = timer.elapsed();

        QByteArray data = f.readAll();
        out.readMs = timer.elapsed() - out.openMs;

        QJsonParseError e;
        QJsonDocument doc = QJsonDocument::fromJson(data, &e);
        out.parseMs = timer.elapsed() - out.openMs - out.readMs;

        if (e.error != QJsonParseError::NoError) {
            out.error = "JSON parse error: " + e.errorString();
            return out;
        }

        // Build model in worker thread (no parent!)
        // We need FileReader's buildJsonTreeModel; that is a member function.
        // To avoid calling into QObject methods across threads, do NOT touch self if it's null.
        if (!self) {
            out.error = "Loader object destroyed";
            return out;
        }

        // IMPORTANT: buildJsonTreeModel must not touch any UI/QML objects.
        QStandardItemModel* model = self->buildJsonTreeModel(doc, nullptr, sortKeys);

        out.buildMs = timer.elapsed() - out.openMs - out.readMs - out.parseMs;
        out.totalMs = timer.elapsed();

        out.model = model;
        out.ok = true;
        return out;
    }));
}

void FileReader::addJsonToParent(QStandardItem* parent, const QString& key, const QJsonValue& value, bool sortObjectKeys)
{
    QStandardItem* keyItem = new QStandardItem(key);
    QStandardItem* valueItem = new QStandardItem(getDebugValueFromType(value));
    parent->appendRow({keyItem, valueItem});

    if (value.isObject())
    {
        const QJsonObject obj = value.toObject();
        QStringList keys = obj.keys();

        if (sortObjectKeys)
        {
            keys.sort(Qt::CaseInsensitive);
        }

        for (const QString &k : keys)
        {
            addJsonToParent(keyItem, k, obj.value(k), sortObjectKeys);
        }
    } else if (value.isArray())
    {
        const QJsonArray arr = value.toArray();

        for (int i = 0; i < arr.size(); ++i)
        {
            addJsonToParent(keyItem, QString("[%1]").arg(i), arr.at(i), sortObjectKeys);
        }
    }
}

QStandardItemModel* FileReader::buildJsonTreeModel(const QJsonDocument& doc, QObject* parent, bool sortObjectKeys)
{
    QStandardItemModel* model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({"Key", "Value"});

    QStandardItem *root = model->invisibleRootItem();

    if (doc.isObject())
    {
        addJsonToParent(root, "$", QJsonValue(doc.object()), sortObjectKeys);
    }
    else if (doc.isArray())
    {
        addJsonToParent(root, "$", QJsonValue(doc.array()), sortObjectKeys);
    }

    // a JSON should always start with obj or array, error can be handled here

    return model;
}

QString FileReader::getJsonTypeAsString(QJsonValue::Type type)
{
    switch (type)
    {
        case QJsonValue::Null:      return "Null";
        case QJsonValue::Bool:      return "Bool";
        case QJsonValue::Double:    return "Double";
        case QJsonValue::String:    return "String";
        case QJsonValue::Array:     return "Array";
        case QJsonValue::Object:    return "Object";
        case QJsonValue::Undefined: return "Undefined";
    }

    return QString("Unknown");
}

QString FileReader::getDebugValueFromType(QJsonValue value)
{
    switch (value.type())
    {
        case QJsonValue::Null:      return "null";
        case QJsonValue::Bool:      return value.toBool() ? "True" : "False";
        case QJsonValue::Double:    return QString::number(value.toDouble());
        case QJsonValue::String:    return value.toString();
        case QJsonValue::Array:     return QString("[ ] (%1)").arg(value.toArray().size());
        case QJsonValue::Object:    return QString("{ } (%1)").arg(value.toObject().size());
        case QJsonValue::Undefined: return "<undefined>";
    }

    return QString("");
}

void FileReader::analyzeJson(QJsonDocument doc)
{
    if (doc.isArray())
    {
        QJsonArray docArray = doc.array();
        for (int i = 0; i < docArray.size(); i++)
        {
            printElementInfo(QString::number(i), docArray[i]);
        }
    }
    else
    {
        QJsonObject docObject = doc.object();
        for (const auto& element : docObject.asKeyValueRange())
        {
            printElementInfo(element.first.toString(), element.second);
        }
    }
}

void FileReader::printElementInfo(QString key, QJsonValue value)
{
    qDebug() << getJsonTypeAsString(value.type()) << key << getDebugValueFromType(value);

    if (value.isObject())
    {
        QJsonObject objElement = value.toObject();

        for (const auto& element : objElement.asKeyValueRange())
        {
            printElementInfo(element.first.toString(), element.second);
        }
    }
    else if (value.isArray())
    {
        QJsonArray arrayElement = value.toArray();

        for (int i = 0; i < arrayElement.size(); i++)
        {
            printElementInfo(QString::number(i), arrayElement[i]);
        }
    }
}

QStandardItemModel* FileReader::jsonModel() const
{
    return m_jsonModel;
}

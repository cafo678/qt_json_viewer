#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>

#include <QJsonValue>
#include <QStandardItemModel>

class QStandardItem;

class FileReader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStandardItemModel* jsonModel READ jsonModel NOTIFY jsonModelChanged FINAL)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit FileReader(QObject *parent = nullptr);

    Q_INVOKABLE void readJsonFile(const QUrl &url);
    Q_INVOKABLE void readJsonFileAsync(const QUrl &url);

    QStandardItemModel* jsonModel() const;
    bool loading() const { return m_loading; }
    QString statusText() const { return m_statusText; }

signals:
    void jsonModelChanged();
    void loadingChanged();
    void statusTextChanged();

private:
    void analyzeJson(QJsonDocument doc);
    void printElementInfo(QString key, QJsonValue value);
    void addJsonToParent(QStandardItem* parent, const QString& key, const QJsonValue& value, bool sortObjectKeys);

    QStandardItemModel* buildJsonTreeModel(const QJsonDocument& doc, QObject* parent = nullptr, bool sortObjectKeys = true);
    QString getJsonTypeAsString(QJsonValue::Type type);
    QString getDebugValueFromType(QJsonValue value);

    void setLoading(bool v);
    void setStatusText(const QString &t);

    QStandardItemModel* m_jsonModel = nullptr;
    bool m_loading = false;
    QString m_statusText;
    bool m_loadInProgress = false;
};

#endif // FILEREADER_H

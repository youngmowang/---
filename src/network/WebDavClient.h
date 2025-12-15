#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class WebDavClient : public QObject {
    Q_OBJECT

public:
    explicit WebDavClient(QObject *parent = nullptr);
    
    void setCredentials(const QString& url, const QString& username, const QString& password);
    void uploadDatabase(const QString& localPath, const QString& remoteName);
    void downloadDatabase(const QString& remoteName, const QString& localPath);

signals:
    void uploadFinished(bool success, QString message);
    void downloadFinished(bool success, QString message);

private:
    QNetworkAccessManager *m_manager;
    QUrl m_url;
    QString m_username;
    QString m_password;

    void onUploadFinished(QNetworkReply *reply);
    void onDownloadFinished(QNetworkReply *reply, const QString& localPath);
};

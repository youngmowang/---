#include "WebDavClient.h"
#include <QFile>
#include <QNetworkRequest>
#include <QAuthenticator>

WebDavClient::WebDavClient(QObject *parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::authenticationRequired, 
            this, [this](QNetworkReply*, QAuthenticator* authenticator){
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
    });
}

void WebDavClient::setCredentials(const QString& url, const QString& username, const QString& password) {
    m_url = QUrl(url);
    if (!m_url.path().endsWith("/")) {
        m_url.setPath(m_url.path() + "/");
    }
    m_username = username;
    m_password = password;
}

void WebDavClient::uploadDatabase(const QString& localPath, const QString& remoteName) {
    QFile *file = new QFile(localPath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadFinished(false, "Could not open local file");
        delete file;
        return;
    }

    QUrl uploadUrl = m_url;
    uploadUrl.setPath(uploadUrl.path() + remoteName);

    QNetworkRequest request(uploadUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QNetworkReply *reply = m_manager->put(request, file);
    file->setParent(reply); 
    
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onUploadFinished(reply);
    });
}

void WebDavClient::downloadDatabase(const QString& remoteName, const QString& localPath) {
    QUrl downloadUrl = m_url;
    downloadUrl.setPath(downloadUrl.path() + remoteName);

    QNetworkRequest request(downloadUrl);
    QNetworkReply *reply = m_manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, localPath](){
        onDownloadFinished(reply, localPath);
    });
}

void WebDavClient::onUploadFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        emit uploadFinished(true, "Upload successful");
    } else {
        emit uploadFinished(false, reply->errorString());
    }
    reply->deleteLater();
}

void WebDavClient::onDownloadFinished(QNetworkReply *reply, const QString& localPath) {
    if (reply->error() == QNetworkReply::NoError) {
        QFile file(localPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            emit downloadFinished(true, "Download successful");
        } else {
            emit downloadFinished(false, "Could not write to local file");
        }
    } else {
        emit downloadFinished(false, reply->errorString());
    }
    reply->deleteLater();
}

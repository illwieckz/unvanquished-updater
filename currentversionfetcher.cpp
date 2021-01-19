#include "currentversionfetcher.h"

#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

CurrentVersionFetcher::CurrentVersionFetcher(QObject* parent) : QObject(parent), manager_(new QNetworkAccessManager(this))
{
    connect(manager_.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(reply(QNetworkReply*)));
}

void CurrentVersionFetcher::fetchCurrentVersion(QString url)
{
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    manager_->get(request);
}

void CurrentVersionFetcher::reply(QNetworkReply* reply)
{
    QString gameVersion;
    QString updaterVersion;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "CurrentVersionFetcher: network error";
        emit onCurrentVersions(updaterVersion, gameVersion);
        return;
    }

    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "CurrentVersionFetcher: JSON parsing error";
        emit onCurrentVersions(updaterVersion, gameVersion);
        return;
    }
    QJsonObject jsonObject = json.object();

    QJsonObject updaterObject = jsonObject["updater"].toObject();
    if (!updaterObject.isEmpty()) {
        QJsonValue version = updaterObject.value("version");
        if (version != QJsonValue::Undefined) {
            updaterVersion = version.toString();
        } else {
            qDebug() << "CurrentVersionFetcher: undefined “version” updater value";
        }
    } else {
        qDebug() << "CurrentVersionFetcher: undefined “updater” key";
    }

    QJsonObject gameObject = jsonObject["game"].toObject();
    if (!gameObject.isEmpty()) {
        QJsonValue version = gameObject.value("version");
        if (version != QJsonValue::Undefined) {
            gameVersion = version.toString();
        } else {
            qDebug() << "CurrentVersionFetcher: undefined “version” game value";
        }
    } else {
        qDebug() << "CurrentVersionFetcher: undefined “game” key";
    }

    qDebug() << "CurrentVersionFetcher: fetched versions: updater =" << updaterVersion << "game =" << gameVersion;

    emit onCurrentVersions(updaterVersion, gameVersion);
}


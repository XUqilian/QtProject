#include <SessionMessage/SessionMessage.h>

SessionMessageTools::SessionMessageTools(QObject *parent) : QObject(parent) {}

bool SessionMessageTools::copyFile(const QUrl &sourceUrl, const QUrl &targetUrl) {
    QString src = sourceUrl.toLocalFile();
    QString tgt = targetUrl.toLocalFile();

    // ✅ 1. 检查源文件存在
    if (!QFile::exists(src)) {
        qWarning() << "源文件不存在:" << src;
        return false;
    }

    // ✅ 2. 如果目标已存在，应由调用者决定（如 FileDialog 已处理）
    //    或在这里用 QFile::remove(tgt) 前确认
    // 计算哈希 但需要完整读取文件readAll 可能会爆内存
    // QCryptographicHash oldHash(QCryptographicHash::Sha256);
    // oldHash.addData(oldData);
    // QCryptographicHash newHash(QCryptographicHash::Sha256);
    // newHash.addData(newData);
    // if (oldHash.result() == newHash.result())

    // ✅ 3. 使用 QFile::copy（底层是系统调用，高效且原子）
    if (QFile::copy(src, tgt)) {
        QFile::setPermissions(tgt, QFile::permissions(src)); // 保持权限
        qDebug() << "复制成功:" << src << "->" << tgt;
        return true;
    } else {
        qWarning() << "复制失败!";
        return false;
    }
}

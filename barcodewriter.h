#ifndef BARCODEWRITER_H
#define BARCODEWRITER_H

#include <QObject>
#include "BarcodeFormat.h"
#include "BitMatrix.h"
#include "MultiFormatWriter.h"


#include <QDebug>
#include <QImage>

namespace ZXingQt {

inline QImage WriteBarcode(QStringView text, ZXing::BarcodeFormat format){

    using namespace ZXing;

    auto writer = MultiFormatWriter(format);
    auto matrix = writer.encode(text.toString().toStdString(), 300, 200);
    auto bitmap = ToMatrix<uint8_t>(matrix);


    return QImage(bitmap.data(), bitmap.width(), bitmap.height(), bitmap.width(), QImage::Format::Format_Grayscale8).copy();

}

}// namespace ZXingQt


class BarcodeWriter : public QObject
{
    Q_OBJECT
public:
    explicit BarcodeWriter(QObject *parent = nullptr)
        : QObject(parent)
    {}

    Q_PROPERTY(QString imageUrl READ imageUrl WRITE setImageUrl NOTIFY imageUrlChanged)

    QString imageUrl() const
    {
        return m_imageUrl;
    }

    void setImageUrl(const QString &newUrl)
    {
        if (newUrl != m_imageUrl) {
            m_imageUrl = newUrl;
            emit imageUrlChanged();
        }
    }

public slots:
    QImage writeAndSaveBarcode(const QString &format, const QString &text)
    {
        if (format.isEmpty() || text.isEmpty()) {
            qDebug() << "Cannot generate barcode out of empty content.";
            return QImage();
        }

        auto barcodeFormat = ZXing::BarcodeFormatFromString(format.toStdString());

        auto result = ZXingQt::WriteBarcode(QStringView(text), barcodeFormat);

        QString filePath = "C:/Users/Admin/Downloads/Barcodes/" + text + ".png";
        result.save(filePath);

        setImageUrl("file:///" + filePath);

        emit barcodeSaved();
        return result;
    }

signals:
    void barcodeSaved();
    void imageUrlChanged();

private:
    QString m_imageUrl;
};

#endif // BARCODEWRITER_H

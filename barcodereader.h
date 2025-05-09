#ifndef BARCODEREADER_H
#define BARCODEREADER_H

#pragma once

#include "ReadBarcode.h"
#include <QImage>
#include <QDebug>
#include <QMetaType>
#include <QScopeGuard>

#include <QVideoFrame>
#include <QVideoSink>
#include <QElapsedTimer>

//for Concurrency
#include <QtConcurrent>
#include <QFutureWatcher>

namespace ZXingQt {

Q_NAMESPACE

#ifdef QT_QML_LIB

enum class BarcodeFormat{

    None            = 0,         ///< Used as a return value if no valid barcode has been detected
    Aztec           = (1 << 0),  ///< Aztec
    Codabar         = (1 << 1),  ///< Codabar
    Code39          = (1 << 2),  ///< Code39
    Code93          = (1 << 3),  ///< Code93
    Code128         = (1 << 4),  ///< Code128
    DataBar         = (1 << 5),  ///< GS1 DataBar, formerly known as RSS 14
    DataBarExpanded = (1 << 6),  ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
    DataMatrix      = (1 << 7),  ///< DataMatrix
    EAN8            = (1 << 8),  ///< EAN-8
    EAN13           = (1 << 9),  ///< EAN-13
    ITF             = (1 << 10), ///< ITF (Interleaved Two of Five)
    MaxiCode        = (1 << 11), ///< MaxiCode
    PDF417          = (1 << 12), ///< PDF417 or
    QRCode          = (1 << 13), ///< QR Code
    UPCA            = (1 << 14), ///< UPC-A
    UPCE            = (1 << 15), ///< UPC-E
    MicroQRCode     = (1 << 16), ///< Micro QR Code
    RMQRCode        = (1 << 17), ///< Rectangular Micro QR Code
    DXFilmEdge      = (1 << 18), ///< DX Film Edge Barcode
    DataBarLimited  = (1 << 19), ///< GS1 DataBar Limited

    LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | DataBarLimited | DXFilmEdge | UPCA | UPCE,
    MatrixCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode | RMQRCode,

};

enum class ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };

enum class TextMode { Plain, ECI, HRI, Hex, Escaped };

#else
using ZXing::BarcodeFormat;
using ZXing::ContentType;
using ZXing::TextMode;
#endif

using ZXing::ReaderOptions;
using ZXing::Binarizer;
using ZXing::BarcodeFormats;

Q_ENUM_NS(BarcodeFormat)
Q_ENUM_NS(ContentType)
Q_ENUM_NS(TextMode)


// << operator overload for converting ZXing objects that have the ToString method to be output by Qt's QDebug()
template<typename T, typename = decltype(ZXing::ToString(T()))>
QDebug operator<<(QDebug dbg, const T& v)
{
    return dbg.noquote() << QString::fromStdString(ToString(v));
}

class Position : public ZXing::Quadrilateral<QPoint>
{
    Q_GADGET //make the class usable by QMeta-object system, enough to expose it to Qml

    Q_PROPERTY(QPoint topLeft READ topLeft)
    Q_PROPERTY(QPoint topRight READ topRight)
    Q_PROPERTY(QPoint bottomRight READ bottomRight)
    Q_PROPERTY(QPoint bottomLeft READ bottomLeft)

    using Base = ZXing::Quadrilateral<QPoint>;

public:
    using Base::Base;
};

class Barcode : private ZXing::Barcode
{
    Q_GADGET

    Q_PROPERTY(BarcodeFormat format READ format)
    Q_PROPERTY(QString formatName READ formatName)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QByteArray bytes READ bytes)
    Q_PROPERTY(bool isValid READ isValid)
    Q_PROPERTY(ContentType contentType READ contentType)
    Q_PROPERTY(QString contentTypeName READ contentTypeName)
    Q_PROPERTY(Position position READ position)

    QString _text;
    QByteArray _bytes;
    Position _position;

public:
    Barcode() = default; // required for qmetatype machinery

    explicit Barcode(ZXing::Barcode&& r) : ZXing::Barcode(std::move(r)) {
        _text = QString::fromStdString(ZXing::Barcode::text());
        _bytes = QByteArray(reinterpret_cast<const char*>(ZXing::Barcode::bytes().data()), Size(ZXing::Barcode::bytes()));
        auto& pos = ZXing::Barcode::position();
        auto qp = [&pos](int i) { return QPoint(pos[i].x, pos[i].y); };
        _position = {qp(0), qp(1), qp(2), qp(3)};
    }

    using ZXing::Barcode::isValid; //inherited directly from base class to check if the Barode is a valid format

    BarcodeFormat format() const { return static_cast<BarcodeFormat>(ZXing::Barcode::format()); }
    ContentType contentType() const { return static_cast<ContentType>(ZXing::Barcode::contentType()); }
    QString formatName() const { return QString::fromStdString(ZXing::ToString(ZXing::Barcode::format())); }
    QString contentTypeName() const { return QString::fromStdString(ZXing::ToString(ZXing::Barcode::contentType())); }
    const QString& text() const { return _text; }
    const QByteArray& bytes() const { return _bytes; }
    const Position& position() const { return _position; }
};

//takes a vector of ZXing barcodes and converts them to our custom Barcode class objects
inline QList<Barcode> ZXBarcodesToQBarcodes(ZXing::Barcodes&& zxres)
{
    QList<Barcode> res;

    for (auto&& r : zxres)
        res.emplace_back(std::move(r));
    return res;
}

inline QList<Barcode> ReadBarcodes(const QImage& img, const ReaderOptions& opts = {}){

    using namespace ZXing;

    auto ImgFmtFromQImg = [](const QImage& img){
        switch (img.format()) {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            return ImageFormat::BGRA;
#else
            return ImageFormat::ARGB;
#endif
        case QImage::Format_RGB888: return ImageFormat::RGB;
        case QImage::Format_RGBX8888:
        case QImage::Format_RGBA8888: return ImageFormat::RGBA;
        case QImage::Format_Grayscale8: return ImageFormat::Lum;
        default: return ImageFormat::None;
        }
    };

    auto exec = [&](const QImage& img){
        return ZXBarcodesToQBarcodes(
            ZXing::ReadBarcodes({img.bits(), img.width(), img.height(), ImgFmtFromQImg(img), static_cast<int>(img.bytesPerLine())}, opts));
    };

    return ImgFmtFromQImg(img) == ImageFormat::None ? exec(img.convertToFormat(QImage::Format_Grayscale8)) : exec(img);
}

//Async version
inline QFuture<QList<Barcode>> ReadBarcodesAsync(const QImage& img, const ReaderOptions& opts = {})
{
    return QtConcurrent::run([img, opts]() {
        return ReadBarcodes(img, opts);
    });
}

inline Barcode ReadBarcode(const QImage& img, const ReaderOptions& opts = {}){
    auto res = ReadBarcodes(img, ReaderOptions(opts).setMaxNumberOfSymbols(1));
    return !res.isEmpty() ? res.takeFirst() : Barcode();
}

//Async Version
inline QFuture<Barcode> ReadBarcodeAsync(const QImage& img, const ReaderOptions& opts = {})
{
    return QtConcurrent::run([img, opts]() {
        return ReadBarcode(img, opts);
    });
}

#ifdef QT_MULTIMEDIA_LIB

const inline QList<Barcode> ReadBarcodes(const QVideoFrame& frame, const ReaderOptions& opts = {}){

    using namespace ZXing;

    ImageFormat fmt = ImageFormat::None;
    int pixStride = 0;
    int pixOffset = 0;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define FORMAT(F5, F6) QVideoFrame::Format_##F5
#define FIRST_PLANE
#else
#define FORMAT(F5, F6) QVideoFrameFormat::Format_##F6
#define FIRST_PLANE 0
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define FORMAT(F5, F6) QVideoFrame::Format_##F5
#define FIRST_PLANE
#else
#define FORMAT(F5, F6) QVideoFrameFormat::Format_##F6
#define FIRST_PLANE 0
#endif

    switch (frame.pixelFormat()) {
    case FORMAT(ARGB32, ARGB8888):
    case FORMAT(ARGB32_Premultiplied, ARGB8888_Premultiplied):
    case FORMAT(RGB32, RGBX8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        fmt = ImageFormat::BGRA;
#else
        fmt = ImageFormat::ARGB;
#endif
        break;

    case FORMAT(BGRA32, BGRA8888):
    case FORMAT(BGRA32_Premultiplied, BGRA8888_Premultiplied):
    case FORMAT(BGR32, BGRX8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        fmt = ImageFormat::RGBA;
#else
        fmt = ImageFormat::ABGR;
#endif
        break;

    case QVideoFrameFormat::Format_P010:
    case QVideoFrameFormat::Format_P016: fmt = ImageFormat::Lum, pixStride = 1; break;

    case FORMAT(AYUV444, AYUV):
    case FORMAT(AYUV444_Premultiplied, AYUV_Premultiplied):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        fmt = ImageFormat::Lum, pixStride = 4, pixOffset = 3;
#else
        fmt = ImageFormat::Lum, pixStride = 4, pixOffset = 2;
#endif
        break;

    case FORMAT(YUV420P, YUV420P):
    case FORMAT(NV12, NV12):
    case FORMAT(NV21, NV21):
    case FORMAT(IMC1, IMC1):
    case FORMAT(IMC2, IMC2):
    case FORMAT(IMC3, IMC3):
    case FORMAT(IMC4, IMC4):
    case FORMAT(YV12, YV12): fmt = ImageFormat::Lum; break;
    case FORMAT(UYVY, UYVY): fmt = ImageFormat::Lum, pixStride = 2, pixOffset = 1; break;
    case FORMAT(YUYV, YUYV): fmt = ImageFormat::Lum, pixStride = 2; break;

    case FORMAT(Y8, Y8): fmt = ImageFormat::Lum; break;
    case FORMAT(Y16, Y16): fmt = ImageFormat::Lum, pixStride = 2, pixOffset = 1; break;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    case FORMAT(ABGR32, ABGR8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        fmt = ImageFormat::RGBA;
#else
        fmt = ImageFormat::ABGR;
#endif
        break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    case FORMAT(YUV422P, YUV422P): fmt = ImageFormat::Lum; break;
#endif
    default: break;
    }

    if (fmt != ImageFormat::None) {
        auto img = frame; // shallow copy just get access to non-const map() function
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (!img.isValid() || !img.map(QAbstractVideoBuffer::ReadOnly)){
#else
        if (!img.isValid() || !img.map(QVideoFrame::ReadOnly)){
#endif
            qWarning() << "invalid QVideoFrame: could not map memory";
            return {};
        }
        QScopeGuard unmap([&] { img.unmap(); });

        return ZXBarcodesToQBarcodes(ZXing::ReadBarcodes(
            {img.bits(FIRST_PLANE) + pixOffset, img.width(), img.height(), fmt, img.bytesPerLine(FIRST_PLANE), pixStride}, opts));
    }
    else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()) != QImage::Format_Invalid) {
            qWarning() << "unsupported QVideoFrame::pixelFormat";
            return {};
        }
        auto qimg = frame.image();
#else
        auto qimg = frame.toImage();
#endif
        if (qimg.format() != QImage::Format_Invalid)
            return ReadBarcodes(qimg, opts);
        qWarning() << "failed to convert QVideoFrame to QImage";
        return {};
    }
}

//Async version
inline QFuture<QList<Barcode>> ReadBarcodesAsync(const QVideoFrame& frame, const ReaderOptions& opts = {})
{
    return QtConcurrent::run([frame, opts]() {
        return ReadBarcodes(frame, opts);
    });
}

inline Barcode ReadBarcode(const QVideoFrame& frame, const ReaderOptions& opts = {}){
    auto res = ReadBarcodes(frame, ReaderOptions(opts).setMaxNumberOfSymbols(1));
    return !res.isEmpty() ? res.takeFirst() : Barcode();
}

//Async Version
inline QFuture<Barcode> ReadBarcodeAsync(const QVideoFrame& frame, const ReaderOptions& opts = {})
{
    return QtConcurrent::run([frame, opts]() {
        return ReadBarcode(frame, opts);
    });
}
//the Async Versions are just wrappers of the orginal function, to prevent the main UI thread from freezing

#define ZQ_PROPERTY(Type, name, setter) \
public: \
    Q_PROPERTY(Type name READ name WRITE setter NOTIFY name##Changed) \
    Type name() const noexcept { return ReaderOptions::name(); } \
    Q_SLOT void setter(const Type& newVal) \
{ \
        if (name() != newVal) { \
            ReaderOptions::setter(newVal); \
            emit name##Changed(); \
    } \
} \
    Q_SIGNAL void name##Changed();


class BarcodeReader: public QObject, private ReaderOptions {

    Q_OBJECT

public:
    BarcodeReader(QObject* parent = nullptr) : QObject(parent) {}

    Q_PROPERTY(int formats READ formats WRITE setFormats NOTIFY formatsChanged)

    int formats() const noexcept{
        auto fmts = ReaderOptions::formats();
        return *reinterpret_cast<int*>(&fmts);
    }

    Q_SLOT void setFormats(int newVal){
        if (formats() != newVal){
            ReaderOptions::setFormats(static_cast<ZXing::BarcodeFormat>(newVal));
            emit formatsChanged();
            qDebug() << ReaderOptions::formats();
        }
    }

    Q_SIGNAL formatsChanged();

    Q_PROPERTY(TextMode textMode READ textMode WRITE setTextMode NOTIFY textModeChanged)
    TextMode textMode() const noexcept { return static_cast<TextMode>(ReaderOptions::textMode()); }
    Q_SLOT void setTextMode(TextMode newVal)
    {
        if (textMode() != newVal) {
            ReaderOptions::setTextMode(static_cast<ZXing::TextMode>(newVal));
            emit textModeChanged();
        }
    }
    Q_SIGNAL void textModeChanged();

    ZQ_PROPERTY(bool, tryRotate, setTryRotate)
    ZQ_PROPERTY(bool, tryHarder, setTryHarder)
    ZQ_PROPERTY(bool, tryInvert, setTryInvert)
    ZQ_PROPERTY(bool, tryDownscale, setTryDownscale)
    ZQ_PROPERTY(bool, isPure, setIsPure)

    // For debugging/development
    int runTime = 0;
    Q_PROPERTY(int runTime MEMBER runTime)

public slots:
    void process(const QVideoFrame& image)
    {
        QElapsedTimer t;
        t.start();

        // Create a copy of the video frame to use in the other thread
        QVideoFrame frameCopy = image;

        // Start barcode detection in a separate thread
        QFuture<Barcode> future = QtConcurrent::run([this, frameCopy]() {
            return ReadBarcode(frameCopy, *this);
        });

        // Create watcher to handle the result
        QFutureWatcher<Barcode>* watcher = new QFutureWatcher<Barcode>(this);

        // Connect to finished signal
        connect(watcher, &QFutureWatcher<Barcode>::finished, this, [this, watcher, t]() {
            Barcode result = watcher->result();
            runTime = t.elapsed();

            if (result.isValid())
                emit foundBarcode(result);
            else
                emit failedRead();

            watcher->deleteLater();
        });

        watcher->setFuture(future);
    }

signals:
    void failedRead();
    void foundBarcode(ZXingQt::Barcode barcode);

private:
    QVideoSink* _sink = nullptr;

public:
    void setVideoSink(QVideoSink* sink) {
        if (_sink == sink)
            return;

        if (_sink)
            disconnect(_sink, nullptr, this, nullptr);

        _sink = sink;
        connect(_sink, &QVideoSink::videoFrameChanged, this, &BarcodeReader::process);
    }
    Q_PROPERTY(QVideoSink* videoSink MEMBER _sink WRITE setVideoSink)
};


#undef ZX_PROPERTY

#endif // QT_MULTIMEDIA_LIB

}//namespace ZXingQt

Q_DECLARE_METATYPE(ZXingQt::Position)
Q_DECLARE_METATYPE(ZXingQt::Barcode)

#ifdef QT_QML_LIB

#include <QQmlEngine>

namespace ZXingQt {

inline void registerQmlAndMetatypes(){
    qRegisterMetaType<ZXingQt::BarcodeFormat>("BarcodeFormat");
    qRegisterMetaType<ZXingQt::ContentType>("ContentType");
    qRegisterMetaType<ZXingQt::TextMode>("TextMode");

    // supposedly the Q_DECLARE_METATYPE should be used with the overload without a custom name
    // but then the qml side complains about "unregistered type"
    qRegisterMetaType<ZXingQt::Position>("Position");
    qRegisterMetaType<ZXingQt::Barcode>("Barcode");

    qmlRegisterUncreatableMetaObject(ZXingQt::staticMetaObject, "ZXing", 1, 0, "ZXing", "Access to enums and flags only");
    qmlRegisterType<ZXingQt::BarcodeReader>("ZXing", 1, 0, "BarcodeReader");
}

} //namespace ZXingQt

#endif //QT_QML_LIB
#endif // BARCODEREADER_H

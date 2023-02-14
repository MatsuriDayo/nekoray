/*
 * Copyright 2020 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ZXing/ReadBarcode.h"

#include <QImage>
#include <QDebug>
#include <QMetaType>
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include "qscopeguard.h"
#else
#include <QScopeGuard>
#endif

#ifdef QT_MULTIMEDIA_LIB
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractVideoFilter>
#else
#include <QVideoFrame>
#include <QVideoSink>
#endif
#include <QElapsedTimer>
#endif

// This is some sample code to start a discussion about how a minimal and header-only Qt wrapper/helper could look like.

namespace ZXingQt {

Q_NAMESPACE

//TODO: find a better way to export these enums to QML than to duplicate their definition
// #ifdef Q_MOC_RUN produces meta information in the moc output but it does end up working in qml
#ifdef QT_QML_LIB
enum class BarcodeFormat
{
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

	LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | UPCA | UPCE,
	MatrixCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode,
};

enum class ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };

#else
using ZXing::BarcodeFormat;
using ZXing::ContentType;
#endif

using ZXing::DecodeHints;
using ZXing::Binarizer;
using ZXing::BarcodeFormats;

Q_ENUM_NS(BarcodeFormat)
Q_ENUM_NS(ContentType)

template<typename T, typename = decltype(ZXing::ToString(T()))>
QDebug operator<<(QDebug dbg, const T& v)
{
	return dbg.noquote() << QString::fromStdString(ToString(v));
}

class Position : public ZXing::Quadrilateral<QPoint>
{
	Q_GADGET

	Q_PROPERTY(QPoint topLeft READ topLeft)
	Q_PROPERTY(QPoint topRight READ topRight)
	Q_PROPERTY(QPoint bottomRight READ bottomRight)
	Q_PROPERTY(QPoint bottomLeft READ bottomLeft)

	using Base = ZXing::Quadrilateral<QPoint>;

public:
	using Base::Base;
};

class Result : private ZXing::Result
{
	Q_GADGET

	Q_PROPERTY(BarcodeFormat format READ format)
	Q_PROPERTY(QString formatName READ formatName)
	Q_PROPERTY(QString text READ text)
	Q_PROPERTY(QByteArray bytes READ bytes)
	Q_PROPERTY(bool isValid READ isValid)
	Q_PROPERTY(ContentType contentType READ contentType)
	Q_PROPERTY(Position position READ position)

	QString _text;
	QByteArray _bytes;
	Position _position;

public:
	Result() = default; // required for qmetatype machinery

	explicit Result(ZXing::Result&& r) : ZXing::Result(std::move(r)) {
		_text = QString::fromStdString(ZXing::Result::text());
		_bytes = QByteArray(reinterpret_cast<const char*>(ZXing::Result::bytes().data()), Size(ZXing::Result::bytes()));
		auto& pos = ZXing::Result::position();
		auto qp = [&pos](int i) { return QPoint(pos[i].x, pos[i].y); };
		_position = {qp(0), qp(1), qp(2), qp(3)};
	}

	using ZXing::Result::isValid;

	BarcodeFormat format() const { return static_cast<BarcodeFormat>(ZXing::Result::format()); }
	ContentType contentType() const { return static_cast<ContentType>(ZXing::Result::contentType()); }
	QString formatName() const { return QString::fromStdString(ZXing::ToString(ZXing::Result::format())); }
	const QString& text() const { return _text; }
	const QByteArray& bytes() const { return _bytes; }
	const Position& position() const { return _position; }

	// For debugging/development
	int runTime = 0;
	Q_PROPERTY(int runTime MEMBER runTime)
};

inline QList<Result> QListResults(ZXing::Results&& zxres)
{
	QList<Result> res;
	for (auto&& r : zxres)
		res.push_back(Result(std::move(r)));
	return res;
}

inline QList<Result> ReadBarcodes(const QImage& img, const DecodeHints& hints = {})
{
	using namespace ZXing;

	auto ImgFmtFromQImg = [](const QImage& img) {
		switch (img.format()) {
		case QImage::Format_ARGB32:
		case QImage::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
			return ImageFormat::BGRX;
#else
			return ImageFormat::XRGB;
#endif
		case QImage::Format_RGB888: return ImageFormat::RGB;
		case QImage::Format_RGBX8888:
		case QImage::Format_RGBA8888: return ImageFormat::RGBX;
		case QImage::Format_Grayscale8: return ImageFormat::Lum;
		default: return ImageFormat::None;
		}
	};

	auto exec = [&](const QImage& img) {
		return QListResults(ZXing::ReadBarcodes(
			{img.bits(), img.width(), img.height(), ImgFmtFromQImg(img), static_cast<int>(img.bytesPerLine())}, hints));
	};

	return ImgFmtFromQImg(img) == ImageFormat::None ? exec(img.convertToFormat(QImage::Format_Grayscale8)) : exec(img);
}

inline Result ReadBarcode(const QImage& img, const DecodeHints& hints = {})
{
	auto res = ReadBarcodes(img, DecodeHints(hints).setMaxNumberOfSymbols(1));
	return !res.isEmpty() ? res.takeFirst() : Result();
}

#ifdef QT_MULTIMEDIA_LIB
inline QList<Result> ReadBarcodes(const QVideoFrame& frame, const DecodeHints& hints = {})
{
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

	switch (frame.pixelFormat()) {
	case FORMAT(ARGB32, ARGB8888):
	case FORMAT(ARGB32_Premultiplied, ARGB8888_Premultiplied):
	case FORMAT(RGB32, RGBX8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::BGRX;
#else
		fmt = ImageFormat::XRGB;
#endif
		break;

	case FORMAT(BGRA32, BGRA8888):
	case FORMAT(BGRA32_Premultiplied, BGRA8888_Premultiplied):
	case FORMAT(BGR32, BGRX8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::RGBX;
#else
		fmt = ImageFormat::XBGR;
#endif
		break;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case QVideoFrame::Format_RGB24: fmt = ImageFormat::RGB; break;
	case QVideoFrame::Format_BGR24: fmt = ImageFormat::BGR; break;
	case QVideoFrame::Format_YUV444: fmt = ImageFormat::Lum, pixStride = 3; break;
#else
	case QVideoFrameFormat::Format_P010:
	case QVideoFrameFormat::Format_P016: fmt = ImageFormat::Lum, pixStride = 1; break;
#endif

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
		fmt = ImageFormat::RGBX;
#else
		fmt = ImageFormat::XBGR;
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

		return QListResults(ZXing::ReadBarcodes(
			{img.bits(FIRST_PLANE) + pixOffset, img.width(), img.height(), fmt, img.bytesPerLine(FIRST_PLANE), pixStride}, hints));
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
			return ReadBarcodes(qimg, hints);
		qWarning() << "failed to convert QVideoFrame to QImage";
		return {};
	}
}

inline Result ReadBarcode(const QVideoFrame& frame, const DecodeHints& hints = {})
{
	auto res = ReadBarcodes(frame, DecodeHints(hints).setMaxNumberOfSymbols(1));
	return !res.isEmpty() ? res.takeFirst() : Result();
}

#define ZQ_PROPERTY(Type, name, setter) \
public: \
	Q_PROPERTY(Type name READ name WRITE setter NOTIFY name##Changed) \
	Type name() const noexcept { return DecodeHints::name(); } \
	Q_SLOT void setter(const Type& newVal) \
	{ \
		if (name() != newVal) { \
			DecodeHints::setter(newVal); \
			emit name##Changed(); \
		} \
	} \
	Q_SIGNAL void name##Changed();


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class BarcodeReader : public QAbstractVideoFilter, private DecodeHints
#else
class BarcodeReader : public QObject, private DecodeHints
#endif
{
	Q_OBJECT

public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	BarcodeReader(QObject* parent = nullptr) : QAbstractVideoFilter(parent) {}
#else
	BarcodeReader(QObject* parent = nullptr) : QObject(parent) {}
#endif

	// TODO: find out how to properly expose QFlags to QML
	// simply using ZQ_PROPERTY(BarcodeFormats, formats, setFormats)
	// results in the runtime error "can't assign int to formats"
	Q_PROPERTY(int formats READ formats WRITE setFormats NOTIFY formatsChanged)
	int formats() const noexcept
	{
		auto fmts = DecodeHints::formats();
		return *reinterpret_cast<int*>(&fmts);
	}
	Q_SLOT void setFormats(int newVal)
	{
		if (formats() != newVal) {
			DecodeHints::setFormats(static_cast<ZXing::BarcodeFormat>(newVal));
			emit formatsChanged();
			qDebug() << DecodeHints::formats();
		}
	}
	Q_SIGNAL void formatsChanged();

	ZQ_PROPERTY(bool, tryRotate, setTryRotate)
	ZQ_PROPERTY(bool, tryHarder, setTryHarder)
	ZQ_PROPERTY(bool, tryDownscale, setTryDownscale)

public slots:
	ZXingQt::Result process(const QVideoFrame& image)
	{
		QElapsedTimer t;
		t.start();

		auto res = ReadBarcode(image, *this);

		res.runTime = t.elapsed();

		emit newResult(res);
		if (res.isValid())
			emit foundBarcode(res);
		return res;
	}

signals:
	void newResult(ZXingQt::Result result);
	void foundBarcode(ZXingQt::Result result);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
public:
	QVideoFilterRunnable *createFilterRunnable() override;
#else
private:
	QVideoSink *_sink = nullptr;

public:
	void setVideoSink(QVideoSink* sink) {
		if (_sink == sink)
			return;

		if (_sink)
			disconnect(_sink, nullptr, this, nullptr);

		_sink = sink;
		connect(_sink, &QVideoSink::videoFrameChanged, this, &BarcodeReader::process);
	}
	Q_PROPERTY(QVideoSink* videoSink WRITE setVideoSink)
#endif

};

#undef ZX_PROPERTY

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class VideoFilterRunnable : public QVideoFilterRunnable
{
	BarcodeReader* _filter = nullptr;

public:
	explicit VideoFilterRunnable(BarcodeReader* filter) : _filter(filter) {}

	QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat& /*surfaceFormat*/, RunFlags /*flags*/) override
	{
		_filter->process(*input);
		return *input;
	}
};

inline QVideoFilterRunnable* BarcodeReader::createFilterRunnable()
{
	return new VideoFilterRunnable(this);
}
#endif

#endif // QT_MULTIMEDIA_LIB

} // namespace ZXingQt


Q_DECLARE_METATYPE(ZXingQt::Position)
Q_DECLARE_METATYPE(ZXingQt::Result)

#ifdef QT_QML_LIB

#include <QQmlEngine>

namespace ZXingQt {

inline void registerQmlAndMetaTypes()
{
	qRegisterMetaType<ZXingQt::BarcodeFormat>("BarcodeFormat");
	qRegisterMetaType<ZXingQt::ContentType>("ContentType");

	// supposedly the Q_DECLARE_METATYPE should be used with the overload without a custom name
	// but then the qml side complains about "unregistered type"
	qRegisterMetaType<ZXingQt::Position>("Position");
	qRegisterMetaType<ZXingQt::Result>("Result");

	qmlRegisterUncreatableMetaObject(
		ZXingQt::staticMetaObject, "ZXing", 1, 0, "ZXing", "Access to enums & flags only");
	qmlRegisterType<ZXingQt::BarcodeReader>("ZXing", 1, 0, "BarcodeReader");
}

} // namespace ZXingQt

#endif // QT_QML_LIB
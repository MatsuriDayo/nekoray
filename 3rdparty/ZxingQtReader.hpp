#pragma once
/*
 * Copyright 2020 Axel Waggershauser
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ZXing/ReadBarcode.h"

#include <QImage>
#include <QDebug>
#include <QMetaType>

#ifdef QT_MULTIMEDIA_LIB
#include <QAbstractVideoFilter>
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
	Aztec           = (1 << 0),  ///< Aztec (2D)
	Codabar         = (1 << 1),  ///< Codabar (1D)
	Code39          = (1 << 2),  ///< Code39 (1D)
	Code93          = (1 << 3),  ///< Code93 (1D)
	Code128         = (1 << 4),  ///< Code128 (1D)
	DataBar         = (1 << 5),  ///< GS1 DataBar, formerly known as RSS 14
	DataBarExpanded = (1 << 6),  ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
	DataMatrix      = (1 << 7),  ///< DataMatrix (2D)
	EAN8            = (1 << 8),  ///< EAN-8 (1D)
	EAN13           = (1 << 9),  ///< EAN-13 (1D)
	ITF             = (1 << 10), ///< ITF (Interleaved Two of Five) (1D)
	MaxiCode        = (1 << 11), ///< MaxiCode (2D)
	PDF417          = (1 << 12), ///< PDF417 (1D) or (2D)
	QRCode          = (1 << 13), ///< QR Code (2D)
	UPCA            = (1 << 14), ///< UPC-A (1D)
	UPCE            = (1 << 15), ///< UPC-E (1D)
	MicroQRCode     = (1 << 16), ///< Micro QR Code (2D)

	OneDCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | UPCA | UPCE,
	TwoDCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode,
};

enum class DecodeStatus
{
	NoError = 0,
	NotFound,
	FormatError,
	ChecksumError,
};
#else
    using ZXing::BarcodeFormat;
    using ZXing::DecodeStatus;
#endif

    using ZXing::DecodeHints;
    using ZXing::Binarizer;
    using ZXing::BarcodeFormats;

    Q_ENUM_NS(BarcodeFormat)
    Q_ENUM_NS(DecodeStatus)

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
        Q_PROPERTY(QByteArray rawBytes READ rawBytes)
        Q_PROPERTY(bool isValid READ isValid)
        Q_PROPERTY(DecodeStatus status READ status)
        Q_PROPERTY(Position position READ position)

        QString _text;
        QByteArray _rawBytes;
        Position _position;

    public:
        Result() : ZXing::Result(ZXing::DecodeStatus::NotFound) {} // required for qmetatype machinery

        explicit Result(ZXing::Result&& r) : ZXing::Result(std::move(r)) {
            _text = QString::fromWCharArray(ZXing::Result::text().c_str());
            _rawBytes = QByteArray(reinterpret_cast<const char*>(ZXing::Result::rawBytes().data()),
                                   Size(ZXing::Result::rawBytes()));
            auto& pos = ZXing::Result::position();
            auto qp = [&pos](int i) { return QPoint(pos[i].x, pos[i].y); };
            _position = {qp(0), qp(1), qp(2), qp(3)};
        }

        using ZXing::Result::isValid;

        BarcodeFormat format() const { return static_cast<BarcodeFormat>(ZXing::Result::format()); }
        DecodeStatus status() const { return static_cast<DecodeStatus>(ZXing::Result::status()); }
        QString formatName() const { return QString::fromStdString(ZXing::ToString(ZXing::Result::format())); }
        const QString& text() const { return _text; }
        const QByteArray& rawBytes() const { return _rawBytes; }
        const Position& position() const { return _position; }

        // For debugging/development
        int runTime = 0;
        Q_PROPERTY(int runTime MEMBER runTime)
    };

    inline Result ReadBarcode(const QImage& img, const DecodeHints& hints = {})
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
            return Result(ZXing::ReadBarcode(
                    {img.bits(), img.width(), img.height(), ImgFmtFromQImg(img), img.bytesPerLine()}, hints));
        };

        return ImgFmtFromQImg(img) == ImageFormat::None ? exec(img.convertToFormat(QImage::Format_Grayscale8)) : exec(img);
    }

#ifdef QT_MULTIMEDIA_LIB
    inline Result ReadBarcode(const QVideoFrame& frame, const DecodeHints& hints = {})
{
	using namespace ZXing;

	auto img = frame; // shallow copy just get access to non-const map() function
	if (!frame.isValid() || !img.map(QAbstractVideoBuffer::ReadOnly)){
		qWarning() << "invalid QVideoFrame: could not map memory";
		return {};
	}
	//TODO c++17:	SCOPE_EXIT([&] { img.unmap(); });

	ImageFormat fmt = ImageFormat::None;
	int pixStride = 0;
	int pixOffset = 0;

	switch (img.pixelFormat()) {
	case QVideoFrame::Format_ARGB32:
	case QVideoFrame::Format_ARGB32_Premultiplied:
	case QVideoFrame::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::BGRX;
#else
		fmt = ImageFormat::XRGB;
#endif
		break;

	case QVideoFrame::Format_RGB24: fmt = ImageFormat::RGB; break;

	case QVideoFrame::Format_BGRA32:
	case QVideoFrame::Format_BGRA32_Premultiplied:
	case QVideoFrame::Format_BGR32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::RGBX;
#else
		fmt = ImageFormat::XBGR;
#endif
		break;

	case QVideoFrame::Format_BGR24: fmt = ImageFormat::BGR; break;

	case QVideoFrame::Format_AYUV444:
	case QVideoFrame::Format_AYUV444_Premultiplied:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::Lum, pixStride = 4, pixOffset = 3;
#else
		fmt = ImageFormat::Lum, pixStride = 4, pixOffset = 2;
#endif
		break;

	case QVideoFrame::Format_YUV444: fmt = ImageFormat::Lum, pixStride = 3; break;
	case QVideoFrame::Format_YUV420P:
	case QVideoFrame::Format_NV12:
	case QVideoFrame::Format_NV21:
	case QVideoFrame::Format_IMC1:
	case QVideoFrame::Format_IMC2:
	case QVideoFrame::Format_IMC3:
	case QVideoFrame::Format_IMC4:
	case QVideoFrame::Format_YV12: fmt = ImageFormat::Lum; break;
	case QVideoFrame::Format_UYVY: fmt = ImageFormat::Lum, pixStride = 2, pixOffset = 1; break;
	case QVideoFrame::Format_YUYV: fmt = ImageFormat::Lum, pixStride = 2; break;

	case QVideoFrame::Format_Y8: fmt = ImageFormat::Lum; break;
	case QVideoFrame::Format_Y16: fmt = ImageFormat::Lum, pixStride = 2, pixOffset = 1; break;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
	case QVideoFrame::Format_ABGR32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::RGBX;
#else
		fmt = ImageFormat::XBGR;
#endif
		break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	case QVideoFrame::Format_YUV422P: fmt = ImageFormat::Lum; break;
#endif
	default: break;
	}

	Result res;
	if (fmt != ImageFormat::None) {
		res = Result(
			ZXing::ReadBarcode({img.bits() + pixOffset, img.width(), img.height(), fmt, img.bytesPerLine(), pixStride},
							   hints));
	} else {
		if (QVideoFrame::imageFormatFromPixelFormat(img.pixelFormat()) != QImage::Format_Invalid)
			res = ReadBarcode(img.image(), hints);
	}

	img.unmap();

	return res;
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

class VideoFilter : public QAbstractVideoFilter, private DecodeHints
{
	Q_OBJECT

public:
	VideoFilter(QObject* parent = nullptr) : QAbstractVideoFilter(parent) {}

	QVideoFilterRunnable* createFilterRunnable() override;

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

public slots:
	Result process(const QVideoFrame& image)
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
	void newResult(Result result);
	void foundBarcode(Result result);
};

#undef ZX_PROPERTY

class VideoFilterRunnable : public QVideoFilterRunnable
{
	VideoFilter* _filter = nullptr;

public:
	explicit VideoFilterRunnable(VideoFilter* filter) : _filter(filter) {}

	QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat& /*surfaceFormat*/, RunFlags /*flags*/) override
	{
		_filter->process(*input);
		return *input;
	}
};

inline QVideoFilterRunnable* VideoFilter::createFilterRunnable()
{
	return new VideoFilterRunnable(this);
}

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
	qRegisterMetaType<ZXingQt::DecodeStatus>("DecodeStatus");

	// supposedly the Q_DECLARE_METATYPE should be used with the overload without a custom name
	// but then the qml side complains about "unregistered type"
	qRegisterMetaType<ZXingQt::Position>("Position");
	qRegisterMetaType<ZXingQt::Result>("Result");

	qmlRegisterUncreatableMetaObject(
		ZXingQt::staticMetaObject, "ZXing", 1, 0, "ZXing", "Access to enums & flags only");
	qmlRegisterType<ZXingQt::VideoFilter>("ZXing", 1, 0, "VideoFilter");
}

} // namespace ZXingQt

#endif // QT_QML_LIB

// for logging purposes (gdcm) we derive from std::ostream, make sure everything is included
#include <string>
#include <exception>
#include <ios>
#include <streambuf>
#include <ostream>
#include <iostream>

// for libtiff logging
#include <cstdio> // sprintf
#include <cstdarg> // variable length func args

#include <modules/dicom/errorlogging.h>

#include <tiff.h>
#include <tiffio.h>

#if defined(IVW_DEBUG)
	// doesnt seem to work, use the cmake GDCM_ALWAYS_TRACE_MACRO flag instead
	#define GDCM_ALWAYS_TRACE_MACRO
#endif

#include <Common/gdcmTrace.h>

namespace inviwo {

// =====================================================================
// gdcm writes its log messages to a std::ostream
// => we derive a logging class from std::ostream and pass
// the logmessages forward to Inviwos LogCentral interface.
// =====================================================================

/* @brief streambuffer class to store log messages and print them at end of line (\n)

no seeking supported!
'simple' output only as no buffer is provided via streambuf::(pub)setbuf()
internal a string is used
*/
template<LogLevel loglevel, class CharT, class Traits = std::char_traits<CharT>>
class streambuf_lineout : public std::streambuf
{
private:
	std::basic_string<CharT, Traits> buffer;

protected:
	// int overflow(int ch) override
	virtual int overflow(int ch)
	{
		try {
			if (ch == '\n') {
				if (false == buffer.empty()) {
					// at the end of a line we want to pass the message to Inviwo's logging system
					inviwo::LogCentral::getPtr()->log("Gdcm Volume Importer", loglevel, LogAudience::User,
						"<GDCM Bibliothek>", "<Funktion>", 0, buffer);
				}
				buffer.clear();
			}
			else if (false == Traits::eq_int_type(ch, Traits::eof())) {
				buffer.push_back(ch);
			}
		}
		catch (std::exception &ex) {
			// error while logging, print what we've got so far
			inviwo::LogCentral::getPtr()->log("Gdcm Volume Importer", loglevel, LogAudience::User,
				"<GDCM Bibliothek>", "<Funktion>", 0, buffer);
			buffer.clear();
			// and tell the user what happened
			LogError("Exception (while logging): " << ex.what());
			// return traits::eof on failure
			return Traits::eof();
		}
		// returning any value != traits::eof is fine on success, so invert eof
		return !(Traits::eof());
	}
};

/* @brief Gdcm log message => std::ostream => Inviwo LogCentral

simple line oriented logging ostream class utilizing streambuf_lineout
not full-featured, /see streambuf_lineout
TODO: check standard conformance
*/
template<LogLevel loglevel>
class ivw_gdcm_log_stream : public std::ostream {
public:
	// std::ios(0) constructor?
	ivw_gdcm_log_stream() : std::ostream(&lnstreambuf) {}

	virtual ~ivw_gdcm_log_stream() {
		// calls sync to clean things up
		lnstreambuf.pubsync();
	}

private:
	streambuf_lineout<loglevel, char, std::char_traits<char>> lnstreambuf;
};

void enable_libgdcm_errorlogging(void) {
	// enable logging in the gdcm library
	static bool logging_enabled = false;
	if (false == logging_enabled) {
		gdcm::Trace::Trace();
#if defined(IVW_DEBUG)
		gdcm::Trace::DebugOn();
		static ivw_gdcm_log_stream<LogLevel::Info> debug_log_stream;
		gdcm::Trace::SetDebugStream(debug_log_stream);
#endif
		gdcm::Trace::WarningOn();
		gdcm::Trace::ErrorOn();
		static ivw_gdcm_log_stream<LogLevel::Warn> warning_log_stream;
		gdcm::Trace::SetWarningStream(warning_log_stream);
		static ivw_gdcm_log_stream<LogLevel::Error> error_log_stream;
		gdcm::Trace::SetErrorStream(error_log_stream);
		logging_enabled = true;

		/*
#if defined(IVW_DEBUG)
		gdcmDebugMacro("GDCM debug output available.");
		gdcmWarningMacro("GDCM warning output available.");
		gdcmErrorMacro("GDCM error output available.");
#endif
		*/
	}
}

// =====================================================================
// logging functionality of libtiff4 - using variable argument lists and snprintf
// =====================================================================

// sprintf for msvc - http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}
#endif

static void mevisLibtiffErrorHandler(const char *module, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char buffer[512];
	int res = snprintf(buffer, 512, fmt, ap);
	va_end(ap);

	std::string err_str;
	if (res < 1) {
		err_str = std::string("got libtiff4 error from ") + std::string(module) + std::string(" but snprintf failed.");
	}
	else {
		err_str = std::string(buffer);
	}

	inviwo::LogCentral::getPtr()->log("Gdcm Volume Importer", LogLevel::Error, LogAudience::User,
		"<libtiff4>", module, 0, err_str);
}

static void mevisLibtiffWarningHandler(const char *module, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char buffer[512];
	int res = snprintf(buffer, 512, fmt, ap);
	va_end(ap);

	std::string err_str;
	if (res < 1) {
		err_str = std::string("got libtiff4 warning from ") + std::string(module) + std::string(" but snprintf failed.");
	}
	else {
		err_str = std::string(buffer);
	}

	inviwo::LogCentral::getPtr()->log("Gdcm Volume Importer", LogLevel::Warn, LogAudience::User,
		"<libtiff4>", module, 0, err_str);
}

void enable_libtiff4_errorlogging(void)
{
	static bool logging_enabled = false;
	if (false == logging_enabled) {
		/*
		TIFFSetErrorHandler((TIFFErrorHandler)mevisLibtiffErrorHandler);
		TIFFSetWarningHandler((TIFFErrorHandler)mevisLibtiffWarningHandler);
		
#if defined(IVW_DEBUG)
		// currently crashes inviwo in 64bit build
		TIFFError("Test", "libtiff error logging available.%s %i %s", "test", 32, "test");
		TIFFWarning("Test", "libtiff warning logging available.");
#endif
		*/
		logging_enabled = true;
	}
}

}

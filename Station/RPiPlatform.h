#ifndef _RPIPLATFORM_H_
#define _RPIPLATFORM_H_

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <cstring>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h> // for mkdir
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */


#define F(d) d
#define PSTR(d) d
#define fprintf_P fprintf

void trace(const char * fmt, ...);
#define trace_setup(...) { ; }
void trace_char(const char c);


#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PROGMEM-compatible definitions

#define prog_char char
#define PROGMEM
#define pgm_read_byte(addr) (*(addr))
#define pgm_read_uint8_t(addr) (*(addr))
#define pgm_read_word_near(addr) (*(addr))
#define __FlashStringHelper char
#define strcpy_P strcpy
#define strcmp_P strcmp
#define sprintf_P sprintf
#define strncmp_P strncmp
#define sscanf_P sscanf

// Standard Print types
#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2


inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline void freeMemory()
{
}

inline int GetFreeMemory()
{
	return -1;
}


class IPAddress
{
private:
	uint8_t _address[4];
public:
	IPAddress();
	IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
	uint8_t* raw_address()
	{
		return _address;
	}

	uint8_t operator[](int index) const
	{
		return _address[index];
	}
	;
	uint8_t& operator[](int index)
	{
		return _address[index];
	}
	;
};

class EEPROMClass
{
public:
	EEPROMClass();
	~EEPROMClass();
	uint8_t read(int addr);
	void write(int addr, uint8_t);
	void Store();
private:
	uint8_t m_buf[4096];
	bool m_changed;
};

extern EEPROMClass EEPROM;

#ifdef INADDR_NONE
#undef INADDR_NONE
#endif
const IPAddress INADDR_NONE(0, 0, 0, 0);

#include <time.h>
class nntp
{
public:
	time_t LocalNow()
	{
		time_t t = time(0);
		struct tm * ti = localtime(&t);
		return t + ti->tm_gmtoff;
	}
	void checkTime()
	{
	}
	bool flagCheckTime()
	{
		return false;
	}
	bool GetNetworkStatus()
	{
		return true;
	}
	void SetLastUpdateTime(void)
	{
		m_LastUpdateTime = time(0);
	}
	
private:
	time_t  m_LastUpdateTime;
};

inline time_t now()
{
		time_t t = time(0);
		struct tm * ti = localtime(&t);
		return t + ti->tm_gmtoff;
}

inline void setTime(time_t newTime)
{
	;
}


// time manipulation functions
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define previousMidnight(_time_) (( _time_ / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  // this is number of days since Jan 1 1970

inline int hour(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_hour;
}

inline int minute(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_min;
}

inline int second(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_sec;
}

inline int year(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_year + 1900;
}

inline int month(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_mon + 1;
}

inline int day(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_mday;
}

inline int weekday(time_t t)
{
	struct tm * ptm;
	ptm = gmtime(&t);
	return ptm->tm_wday + 1;
}

class EthernetServer;

class EthernetClient
{
public:
	EthernetClient();
	EthernetClient(int sock);
	~EthernetClient();
	int connect(IPAddress ip, uint16_t port);
	bool connected();
	void stop();
	int read(uint8_t *buf, size_t size);
	size_t write(const uint8_t *buf, size_t size);
	operator bool();
	int GetSocket()
	{
		return m_sock;
	}
private:
	int m_sock;
	bool m_connected;
	friend class EthernetServer;
};

class EthernetServer
{
public:
	EthernetServer(uint16_t port);
	~EthernetServer();

	bool begin();
	EthernetClient available();

	uint32_t  localIP();
	uint32_t  gatewayIP();
private:
	uint16_t m_port;
	int m_sock;
};

extern EthernetServer *pEthernet;

uint8_t const O_READ = 0X01;
class SdFile
{
public:
	SdFile();
	~SdFile();
	bool open(const char* path, uint32_t oflag = O_READ);
	bool isFile() const;
	bool close();
	bool available();
	int read(void* buf, size_t nbyte);
	int fgets(char *buf, size_t buflen);
	int print(const char *str);
	int println(const char *str);
	int write(char c);
	int write(const char *buf, size_t buflen);
	int seekSet(uint32_t pos)
	{
		if( m_fid == NULL ) return 0;
		fseek(m_fid,pos,SEEK_SET);
		return 1;
	}

	bool isDir()
	{
		return false;
	}

	bool isOpen()
	{
		return m_fid != 0;
	}


private:
	FILE * m_fid;
};

#define O_WRITE	O_RDWR

class SdFat
{
public:
	SdFat();
	~SdFat();

	bool mkdir(const char *str);


};

class Stream
{
public:
    virtual int	 available() = 0;
    virtual int	 read() = 0;
    virtual void flush() = 0;
	virtual void write(uint8_t) = 0;

};

class SerialClass : public Stream
{
public:
	SerialClass();
	~SerialClass();

	bool	begin(int speed);
    int		available();
    int		read();
    void	flush();
	void	write(uint8_t);

private:
	int		port;

};

#endif //_RPIPLATFORM_H_

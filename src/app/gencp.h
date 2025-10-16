#ifndef GENCP_H
#define GENCP_H

#include <stdint.h>
#include "config.h"

#ifdef MODULE_GENCP

#define UART_GENCP UART_CMD

#define DALSA

#define SUPPORTED     1
#define NOT_SUPPORTED 0

#define ENABLE  1
#define DISABLE 0

#define GENCP_XML_HASH_LEN          20 /* sha1-hash */

#define GENCP_XML_USE_NOR_FLASH     1 /* 0: read xml from global array; 1: read xml from nor flash */
#define GENCP_XML_WRITE_TO_FLASH    0 /* write xml to flash */

/* gencp protocol v1.3 */
#define GENCP_VERSION_MINOR    0
#define GENCP_VERSION_MAJOR    1

#define GENCP_CCD_STANDARD_CMD_NUM  6

/* update fpga and xml */
#define U3V_UPDATE_FILE_SELECTOR_FPGA 0
#define U3V_UPDATE_FILE_SELECTOR_XML  1
#define U3V_UPDATE_FILE_SELECTOR_DPC  2
enum {
	U3V_UPDATE_FILE_OP_SELECTOR_OPEN = 0,
	U3V_UPDATE_FILE_OP_SELECTOR_CLOSE,
	U3V_UPDATE_FILE_OP_SELECTOR_READ,
	U3V_UPDATE_FILE_OP_SELECTOR_WRITE,
	U3V_UPDATE_FILE_OP_SELECTOR_DELETE
};
#define U3V_MAX_TRANSFER_LEN      256
#define U3V_FILE_UPDATE_DATA_LEN  (U3V_MAX_TRANSFER_LEN / 2)

/* map zones */
#define GENCP_USER_REG_START_ADDR         	 		0xA00 /* user reg defined in gencp protocol */
#define GENCP_USER_REG_END_ADDR           	 		0xBC0
#define GENCP_SBRM_VIRTUAL_ADDR           	 		0x1000
#define GENCP_MANIFEST_TABLE_VIRTUAL_ADDR 	 		0x2000
#define GENCP_SIRM_VIRTUAL_ADDR           	 		0x3000
#define GENCP_XML_VIRTUAL_START_ADDR         		0x20000 /* 64K is enough for xml file content */
#define GENCP_XML_REG_VIRTUAL_START_ADDR     		0x30000 /* 40K is enough for all of the registers in xml */
#define GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR     0x3A000 /* 16K is enough for all of the user registers in xml */
#define GENCP_XML_UPDATE_REG_VIRTUAL_END_ADDR       0x3E000
#define GENCP_UPDATE_FILE_BUF_VIRTUAL_START_ADDR    0x3E000 /* 8K is enough for u3v file access buffer in xml */
#define GENCP_UPDATE_FILE_BUF_VIRTUAL_END_ADDR  	0x40000
#define GENCP_MANIFEST_ENTRY_NUM          	 		1 /* temporary use 1 */

/* specified by the xml of customer */
/* general xml register */
/* image format control */
#define GENCP_USER_IMAGE_REG_EDITION	 	 0xA00
#define GENCP_USER_IMAGE_REG_RESERVED		 0xA3C
#define GENCP_USER_IMAGE_REG_WIDTH			 0xC04
#define GENCP_USER_IMAGE_REG_HEIGHT			 0xC00
#define GENCP_USER_IMAGE_REG_PIXEL_FORMAT	 0xC08
/* acquisition control */
#ifdef ENDOSCOPE
#define GENCP_USER_IMAGE_TRIGGER_MODE        0xDA0
#define GENCP_USER_IMAGE_ACQ_MODE	         0xDA4
#define GENCP_USER_IMAGE_ACQ_FRAME_CNT       0xDA8
#define GENCP_USER_IMAGE_ACQ_FLAG_EN         0xDAC
#endif

#define GENCP_USER_IMAGE_REG_START_EN        0xBC0
#define GENCP_USER_IMAGE_REG_TEMPERATURE	 0xBC4
#define GENCP_USER_IMAGE_REG_TEMPE_BIAS 	 0xBD8
/* sensor control */
#ifdef CAM2929
#define GENCP_USER_SENSOR_PGA_GAIN			 0xE00
#define GENCP_USER_SENSOR_DIGI_GAIN			 0xE04
#define GENCP_USER_SENSOR_TEST_IMAGE		 0xE08
#define GENCP_USER_SENSOR_X_MIRROR			 0xE0C
#define GENCP_USER_SENSOR_Y_MIRROR			 0xE10
#endif
/* transport layer control */
#define GENCP_USER_IMAGE_REG_PAYLOAD_SIZE	 0xC40
/* FFC */
#define GENCP_USER_IMAGE_CMD_FFC_DARK_CALIB     0xB94
#define GENCP_USER_IMAGE_CMD_FFC_BRIGHT_CALIB   0xB98
/* device control */
#define GENCP_USER_IMAGE_CMD_SELECT_ARG_MODE    0xD40
#define GENCP_USER_IMAGE_CMD_SAVE_ARG           0xD44
#define GENCP_USER_IMAGE_CMD_LOAD_ARG           0xD48
/* mb version */
#define GENCP_USER_IMAGE_CMD_MB_VERSION			0xA10
#define GENCP_USER_IMAGE_CMD_SERIES_NUM			0xA30

#define GENCP_DEV_RESPONSE_TIMEOUT  2000
#define GENCP_HEARTBEAT_TIMEOUT     2000

/* pixel format value according to GenICamPixelFormatValues.pdf */
#define GENCP_PIXEL_FORMAT_MONO8      0x01080001
#define GENCP_PIXEL_FORMAT_MONO10     0x01100003
#define GENCP_PIXEL_FORMAT_MONO10P    0x010A0046
#define GENCP_PIXEL_FORMAT_MONO12     0x01100005
#define GENCP_PIXEL_FORMAT_MONO12P    0x010C0047
#define GENCP_PIXEL_FORMAT_MONO14     0x01100025
#define GENCP_PIXEL_FORMAT_MONO14P    0x010E0104
#define GENCP_PIXEL_FORMAT_MONO16     0x01100007

enum {
    GENCP_CCD_CMDID_CMD = 0,
    GENCP_CCD_CMDID_ACK = 1
};

// #define GENCP_PACKET_HEAD   0x1111
// #define GENCP_PREFIX_PREAMBLE   0x0100
#define GENCP_PREFIX_PREAMBLE   0x43563355
#define GENCP_DAT_LEN_MAX       1024

enum {
    // GENCP_CHK_PREAMBLE_ERR = 0x47430001,
    GENCP_CHK_PREAMBLE_ERR = 0x01000001,
    GENCP_CHK_CCD_CRC_ERR,
    GENCP_CHK_SCD_CRC_ERR,
    GENCP_CHK_ACK_FLAG_ERR,
    GENCP_CHK_PARA_ERR,
    GENCP_XML_CHK_PARA_ERR,
    GENCP_XML_CHK_CRC_ERR
};

/* gencp register map, table19 in gencp v1.3 */
#if 1
#define GENCP_REG_MAP_MANUFACTURE_NAME    ("Aimuon")
#define GENCP_REG_MAP_MODEL_NAME        ("4002")
#define GENCP_REG_MAP_FAMILY_NAME        ("PV")
#define GENCP_REG_MAP_DEV_VER            ("1.0")
#define GENCP_REG_MAP_MANUFACTURE_INFO    ("Aimuon")
#define GENCP_REG_MAP_SERIAL_NUM        ("12345678")
#define GENCP_REG_MAP_USER_DEFINED_NAME    ("PV4002")
#define GENCP_REG_MAP_DEV_SOFT_IF_VER    ("Version1.0")
#else
#define GENCP_REG_MAP_MANUFACTURE_NAME    ("Teledyne_DALSA")
#define GENCP_REG_MAP_MODEL_NAME        ("LA_CM_08K08A_00_R")
#define GENCP_REG_MAP_FAMILY_NAME        ("LA_CL_HARMONY")
#define GENCP_REG_MAP_DEV_VER            ("03-110-20329-02")
#define GENCP_REG_MAP_MANUFACTURE_INFO    ("Aimuon")
#define GENCP_REG_MAP_SERIAL_NUM        ("12128965")
#define GENCP_REG_MAP_USER_DEFINED_NAME    ("PV4002")
#define GENCP_REG_MAP_DEV_SOFT_IF_VER    ("Version1.0")

#endif

/* Gencp ccd ack status code */
#define GENCP_ACK_SUCCESS             (0x0000)
#define GENCP_ACK_NOT_IMPLEMENTED     (0x8001)
#define GENCP_ACK_INVALID_PARAMETER   (0x8002)
#define GENCP_ACK_INVALID_ADDRESS     (0x8003)
#define GENCP_ACK_WRITE_PROTECT       (0x8004)
#define GENCP_ACK_BAD_ALIGNMENT       (0x8005)
#define GENCP_ACK_ACCESS_DENIED       (0x8006)
#define GENCP_ACK_BUSY                (0x8007)
#define GENCP_ACK_MSG_TIMEOUT         (0x800B)
#define GENCP_ACK_INVALID_HEADER      (0x800E)
#define GENCP_ACK_WRONG_CONFIG        (0x800F)
#define GENCP_ACK_ERROR               (0x8FFF)

/* Gencp ccd standard command id */
#define GENCP_READMEM_CMD             (0x0800)
#define GENCP_READMEM_ACK             (0x0801)
#define GENCP_WRITEMEM_CMD            (0x0802)
#define GENCP_WRITEMEM_ACK            (0x0803)
#define GENCP_PENDING_ACK             (0x0805)
#define GENCP_READMEM_STACKED_CMD     (0x0806)
#define GENCP_READMEM_STACKED_ACK     (0x0807)
#define GENCP_WRITEMEM_STACKED_CMD    (0x0808)
#define GENCP_WRITEMEM_STACKED_ACK    (0x0809)
#define GENCP_EVENT_CMD               (0x0C00)
#define GENCP_EVENT_ACK               (0x0C01)
/* Gencp ccd custom command id */
#define GENCP_WRITE_REG_CMD           (0x8800)
#define GENCP_WRITE_REG_ACK           (0x8801)

/* Gencp event id codes */
#define GENCP_EVENT_ID_ERR            (0x0000)

/* Gencp bootstrap register map */
enum {
    GENCP_BAUDRATE_9600 = 0x00000001,
    GENCP_BAUDRATE_19200 = 0x00000002,
    GENCP_BAUDRATE_38400 = 0x00000004,
    GENCP_BAUDRATE_57600 = 0x00000008,
    GENCP_BAUDRATE_115200 = 0x00000010,
    GENCP_BAUDRATE_230400 = 0x00000020,
    GENCP_BAUDRATE_460800 = 0x00000040,
    GENCP_BAUDRATE_921600 = 0x00000080
};

enum {
    GENCP_STRING_ENCODE_ASCII = 0x0,
    GENCP_STRING_ENCODE_UTF8 = 0x1,
    GENCP_STRING_ENCODE_UTF16 = 0x2
};

enum {
    GENCP_ACCESS_PRIVILEGE_AVAILABLE = 0,
    GENCP_ACCESS_PRIVILEGE_OPEN
};

enum {
    GENCP_IMPLEMENT_ENDIAN_BIG = 0,
    GENCP_IMPLEMENT_ENDIAN_LITTLE = 0xFFFFFFFF
};

enum {
    GENCP_MANIFEST_FILE_TYPE_DEV_XML = 0, /* normal */
    GENCP_MANIFEST_FILE_TYPE_BUF_XML
};

enum {
    GENCP_MANIFEST_FILE_FORMAT_UNZIP = 0,
    GENCP_MANIFEST_FILE_FORMAT_ZIP
};

#define GENCP_MANIFEST_TABLE_XML_ADDR    (g_gencpXmlFile.xmlFile)
#define GENCP_MANIFEST_TABLE_XML_SIZE   (g_gencpXmlFile.len)

/* ---------- gencp prefix struct ---------- */
#if 0
typedef struct {
    uint16_t preamble;   /* 0x0100 */
    uint16_t ccdCrc16;   /* ccd crc, channel_id and ccd */
    uint16_t scdCrc16;   /* scd crc, channel_id ccd and scd */
    uint16_t chnId;      /* reserved */
} __attribute__((__packed__)) GencpPrefix;
#else
typedef struct {
    uint32_t preamble;   /* 0x43563355 */
} __attribute__((__packed__)) GencpPrefix;
#endif

/* ---------- gencp ccd struct ---------- */
/* flag in ccd of Gencp */
typedef struct {
    uint16_t reserve : 14; /* reserved */
    uint16_t reqAck : 1;
    uint16_t cmdResend : 1;
} GencpCcdCmdFlag;

/* command id in ccd of Gencp */
typedef struct {
    uint16_t ackFlag : 1;  /* 0: cmd; 1: ack */
    uint16_t cmdValue : 14; /* Number identifying a single command/acknowledge */
    uint16_t custom : 1;  /* 0: standard cmd value; 1: custom cmd value */
} GencpCcdCmdId;

/* gencp ccd cmd */
typedef struct {
    union {
        GencpCcdCmdFlag flag;
        uint16_t flagDat;
    }cmdFlag;

    union {
        GencpCcdCmdId id;
        uint16_t idDat;
    }cmdId;

    uint16_t len;   /* length of SCD */
    uint16_t reqId; /* sequential num of each id, increase 1 of each new cmd */
} __attribute__((__packed__)) GencpCcdCmd;

typedef struct {
    uint16_t statusCode : 12;
    uint16_t reserved : 1;
    uint16_t nameSpace : 2; /* 0: Gencp status code; 1: technology specific code; 2: device specific code */
    uint16_t severity : 1; /* 0: warning/info; 1: error */
} GencpCcdAckStaCode;

/* gencp ccd ack */
typedef struct {
    union {
        GencpCcdAckStaCode code;
        uint16_t codeDat;
    }staCode;
    union {
        GencpCcdCmdId id;
        uint16_t idDat;
    }cmdId;

    uint16_t len; /* length of SCD */
    uint16_t reqId; /* sequential num of each id, increase 1 of each new cmd */
} __attribute__((__packed__)) GencpCcdAck;

/* ---------- gencp scd struct ---------- */
typedef struct {
    uint64_t regAddr;
    uint16_t reserved;
    uint16_t readLen;
} __attribute__((__packed__)) GencpScdReadMemCmd;

typedef struct {
    uint8_t *readDat;
} __attribute__((__packed__)) GencpScdReadMemAck;

typedef struct {
    uint64_t regAddr;
    uint8_t *writeDat;
} __attribute__((__packed__)) GencpScdWriteMemCmd;

typedef struct {
    uint16_t reserved;
    uint16_t writeLen;
} __attribute__((__packed__)) GencpScdWriteMemAck;

typedef struct {
    uint16_t reserved;
    uint16_t tmpTimeout;
} __attribute__((__packed__)) GencpScdPendAck;

typedef struct {
    uint64_t regAddr;
    uint16_t reserved;
    uint16_t readLen;
} __attribute__((__packed__)) GencpScdReadMemStackCmd;

typedef struct {
    uint64_t regAddr;
    uint16_t reserved;
    uint16_t writeLen;
} __attribute__((__packed__)) GencpScdWriteMemStackCmd;

typedef struct {
    uint16_t reserved;
    uint16_t len;
} __attribute__((__packed__)) GencpScdWriteMemStackAck;

typedef struct {
    uint16_t event : 12;
    uint16_t reserved : 2;
    uint16_t namespace : 2;
} EventId;

typedef struct {
    uint16_t eventSize;
    EventId evendId;
    uint64_t timestamp;
} __attribute__((__packed__)) GencpScdEventCmd;

/* gencp cmd data */
typedef struct {
    GencpPrefix prefix;
    GencpCcdCmd ccd;
    uint8_t *scd; /* the len of scd is varialbe */
} __attribute__((__packed__)) GencpCmd;

/* gencp ack data */
typedef struct {
    GencpPrefix prefix;
    GencpCcdAck ccd;
    uint8_t *scd; /* the len of scd is variable */
} __attribute__((__packed__)) GencpAck;


/* ---------- u3v streaming interface register map struct ---------- */
#define FILE_SELECTOR_REG_ADDR       (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR)
#define FILE_OP_SELECTOR_REG_ADDR    (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x4)
#define FILE_OPEN_MODE_REG_ADDR      (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x8)
#define FILE_OP_EXEC_REG_ADDR        (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0xC)
#define FILE_ACCESS_OFFSET_REG_ADDR  (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x14)
#define FILE_ACCESS_LEN_REG_ADDR     (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x18)
#define FILE_OP_STATUS_LEN_REG_ADDR  (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x1C)
#define FILE_OP_RESULT_REG_ADDR      (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x20)
#define FILE_SIZE_REG_ADDR           (GENCP_XML_UPDATE_REG_VIRTUAL_START_ADDR + 0x24)

typedef struct {
    uint32_t reserved : 24;
    uint32_t payloadSizeAlign : 8;
} SiInfo;

typedef struct {
    uint32_t streamEnable : 1; /* default 1 */
    uint32_t reserved : 31;
} SiCtrl;

typedef struct {
    SiInfo siInfo; /* default 0 which mean 2^0 to align */
    SiCtrl siCtrl;
    uint64_t siReqPayloadSize; /* 1 */
    uint32_t siReqLeaderSize; /* 1 */
    uint32_t siReqTrailerSize;  /* 1 */
    uint32_t siMaxLeaderSize; /* 1024 */
    uint32_t siPayloadTransferSize; /* 1024 */
    uint32_t siPayloadTransferCnt; /* 1024 */
    uint32_t siPayloadfinalTransfer1Size; /* 1024 */
    uint32_t siPayloadfinalTransfer2Size; /* 1024 */
    uint32_t siMaxTrailerSize; /* 1024 */
} __attribute__((__packed__)) U3vStreamInterfaceRegMap;

/* sfnc file access control */
typedef struct {
    uint32_t fileSelector;
    uint32_t fileOpSelector;
    uint32_t fileOpenMode;
    uint32_t fileOpExec;
    uint32_t fileAccessBuf;
    uint32_t fileAccessOffset;
    uint32_t fileAccessLen;
    uint32_t fileOpStatus;
    uint32_t fileOpRet;
    uint32_t fileSize;
    uint8_t deviceModelName[32];
    uint32_t fileTotalLen; /* total len of fpga bin file or xml file */
} __attribute__((__packed__)) U3vFileAccessCtrlItem;


/* ---------- gencp bootstrap register map struct ---------- */
#if 0
typedef struct {
    uint32_t supportBaudrate;
    uint32_t currentBaudrate;
} __attribute__((__packed__)) GencpSpecifyBootstrapRegMap;
#else
typedef struct {
    uint32_t u3vVersion;
    uint64_t u3vcpCapabilityReg;
    uint64_t u3vcpConfigReg;
    uint32_t maxCmdTransferLen; /* 1024 */
    uint32_t maxAckTransferLen; /* 1024 */
    uint32_t streamChannelNum; /* 1 */
    uint64_t sirmAddr; /* points to U3vStreamInterfaceRegMap */
    uint32_t sirmLen; /* U3vStreamInterfaceRegMap length */
    uint64_t eirmAddr; /* not used */
    uint32_t eirmLen; /* not used */
    uint64_t iidc2Addr; /* not used */
    uint32_t currentSpeed; /* 4, bit0: low, bit1: full, bit2: high, bit3: SuperSpeed/Gen1/5 Gbps, bit4: SuperSpeedPlus/gen2/10Gbps */
    /* reserved, 65468 bytes */
} __attribute__((__packed__)) GencpSpecifyBootstrapRegMap;
#endif

typedef struct {
    uint32_t major : 16;
    uint32_t minor : 16;
} GencpVersion;

typedef struct {
    uint64_t userDefinedNameSupport : 1;
    uint64_t accessPrivilegeSupport : 1;
    uint64_t msgChnSupport : 1;
    uint64_t timestampSupport : 1;
    uint64_t stringEncode : 4;
    uint64_t familyNameSupport : 1;
    uint64_t sbrmSupport : 1;
    uint64_t endianessRegSupport : 1;
    uint64_t writtenLenFieldSupport : 1;
    uint64_t multiEventSupport : 1;
    uint64_t stackCmdSupport : 1;
    uint64_t devSoftInterfaceVerSupport : 1;
    uint64_t reserved : 49;
} DeviceCapability;

typedef struct {
    uint32_t genicamFileVersionSubMinor : 16;
    uint32_t genicamFileVersionMinor : 8;
    uint32_t genicamFileVersionMajor : 8;
} GencpManifestFileVersion;

typedef struct {
    uint32_t fileType : 3;
    uint32_t reserved0 : 7;
    uint32_t fileFormat : 6; /* 0: uncompressed XML file, 1: zip XML file */
    uint32_t schemaVersionMinor : 8;
    uint32_t schemaVersionMajor : 8;
} GencpManifestFileFormat;

typedef struct {
    union {
        GencpManifestFileVersion fileVersion;
        uint32_t fileVersionDat;
    };

    union {
        GencpManifestFileFormat fileFormat;
        uint32_t fileFormatDat;
    };

    uint64_t registerAddress;
    uint64_t fileSize;
    uint8_t hashSha1[GENCP_XML_HASH_LEN]; /* xml zip file hash */
    uint8_t reserved1[20];
} GencpManifestEntry;

typedef struct {
    uint64_t mtEntryCount;
    GencpManifestEntry entry[GENCP_MANIFEST_ENTRY_NUM];
} __attribute__((__packed__)) GencpManifestTable;

typedef struct {
    uint64_t heartbeatEnable : 1;
    uint64_t multiEventEnable : 1;
    uint64_t reserved : 62;
} DeviceConfig;

/* table19 in gencp v1.3 */
typedef struct {
    GencpVersion gencpVersion;             /* [M]  (R)     0x0000 */
    uint8_t  manufacturerName[64];         /* [M]  (R)     0x0004 */
    uint8_t  modelName[64];                /* [M]  (R)     0x0044 */
    uint8_t  familyName[64];               /* [CM] (R)     0x0084 */
    uint8_t  deviceVersion[64];            /* [M]  (R)     0x00C4 */
    uint8_t  manufacturerInfo[64];         /* [M]  (R)     0x0104 */
    uint8_t  serialNumber[64];             /* [M]  (R)     0x0144 */
    uint8_t  userDefinedName[64];          /* [CM] (RW)    0x0184 */
    DeviceCapability deviceCapability;     /* [M]  (R)     0x01C4 */
    uint32_t maxDeviceResponseTime;        /* [M]  (R)     0x01CC ,not exceed 300ms */
    uint64_t manifestTableAddr;            /* [M]  (R)     0x01D0 */
    uint64_t sbrmAddr;                     /* [CM] (R)     0x01D8 */
    DeviceConfig deviceConfig;             /* [M]  (RW)    0x01E0 */
    uint32_t heartbeatTimeout;             /* [CM] (RW)    0x01E8 */
    uint32_t messageChnId;                 /* [CM] (RW)    0x01EC */
    uint64_t timestamp;                    /* [CM] (R)     0x01F0 */
    uint32_t timestampLatch;               /* [CM] (W)     0x01F8 */
    uint64_t timestampIncrement;           /* [CM] (R)     0x01FC */
    uint32_t accessPrivilege;              /* [CM] (RW)    0x0204 */
    uint32_t protocolEndianess;            /*              0x0208 ,deprecated, ignored */
    uint32_t implementEndianess;           /* [CM] (R)     0x020C */
    uint8_t  devSoftInterfaceVersion[64];  /* [CM] (R)     0x0210 */
    /* reserved 64944 Bytes, do not use */
} __attribute__((__packed__)) GencpRegMap;

typedef struct {
    uint8_t deviceModelName[32];
    uint8_t deviceSerialNumber[16];
} __attribute__((__packed__)) DeviceControl;

typedef struct {
    DeviceControl devCtrl;
    /* TODO: add other information of xml here */
} __attribute__((__packed__)) GencpCustomRegMap;

typedef struct {
    uint64_t cmdStartAddr;
    uint64_t cmdEndAddr;
    uint64_t devStartAddr;
} GencpHostToDevRegMap;

/* map:
0 ~ sizeof(g_gencpRegMap): gencp reg map in gencp V1.3
sizeof(g_gencpRegMap) ~ sizeof(g_gencpRegMap) + sizeof(g_gencpCustomRegMap): user specified reg map
0x0800 ~ 0x09A4: user specified reg map in xml
0x1000 ~ 0x1000 + sizeof(g_gencpSbrm): sbrm, points to baudrate
0x2000 ~ 0x2000 + sizeof(g_gencpManifestTable): manifest table, points to xml
0x3000 ~ 0x3000 + sizeof(g_u3vStreamInterfaceRegMap): sirm, points to the first Streaming Interface Register Map
0x020000 ~ 0x020000 + sizeof(xml): gencp xml
0x030000 ~ 0x03A000: reg map in xml for common reg
0x03A000 ~ 0x03E000: reg map in xml of file access control for update
0x03E000 ~ 0x040000: reg map in xml for file access buffer
*/
enum {
    GENCP_REG_MAP_ZONE_PROTOCOL = 0,
    GENCP_REG_MAP_ZONE_CUSTOM,
    GENCP_REG_MAP_ZONE_USER_XML_REG,
    GENCP_REG_MAP_ZONE_SBRM,
    GENCP_REG_MAP_ZONE_MANIFEST_TABLE,
    GENCP_REG_MAP_ZONE_SIRM,
    GENCP_REG_MAP_ZONE_XML_FILE,
    GENCP_REG_MAP_ZONE_CAMERA_CONFIG,
	GENCP_REG_MAP_ZONE_UPDATE_REG, /* firmware update registers */
	GENCP_REG_MAP_ZONE_UPDATE_DATA,
    GENCP_REG_MAP_ZONE_TOTAL_NUM
};

typedef struct {
    GencpHostToDevRegMap zone[GENCP_REG_MAP_ZONE_TOTAL_NUM];
} GencpRegMapZone;

typedef struct {
    uint16_t cmdId;
    uint16_t (*func)(uint8_t *buf, uint8_t *ackBuf, uint16_t *ackLen);
} GencpCmdCb; /* gencp cmd callback function struct */

typedef struct {
    uint32_t len;
    uint8_t hashSha1[GENCP_XML_HASH_LEN];
    uint8_t *xmlFile;
} GencpXmlFile;

typedef struct {
	/* put all of the data that need to be read periodically here */
	int temp;
} GencpPeriodData;

extern GencpRegMap g_gencpRegMap;
extern GencpCustomRegMap g_gencpCustomRegMap;
extern GencpManifestTable g_gencpManifestTable;
extern GencpSpecifyBootstrapRegMap g_gencpSbrm;
extern GencpXmlFile g_gencpXmlFile;
#if (GENCP_XML_USE_NOR_FLASH == 0)
extern uint8_t gencp_xml_file[];
#endif

int ParseGencpRecvPkg(uint8_t *buf, uint32_t len);
int ParseGencpXmlRecvPkg(uint8_t *buf, uint32_t len);
void InitGencp(void);
void GencpWriteXmlFromArray(void);
int HasXmlInFlash(void);
void GencpGetPeriodData(void);

#endif /* MODULE_GENCP */

#endif /* GENCP_H */

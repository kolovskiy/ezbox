/* ============================================================================
 * Project Name : ezbox configuration utilities
 * File Name    : ezcfg-sys.h
 *
 * Description  : interface to configurate ezbox information
 *
 * Copyright (C) 2008-2012 by ezbox-project
 *
 * History      Rev       Description
 * 2010-11-17   0.1       Write it from scratch
 * ============================================================================
 */

#ifndef _EZCFG_SYS_H_
#define _EZCFG_SYS_H_

/* ezcfg nvram name prefix */
#define EZCFG_SYS_NVRAM_PREFIX        "sys."

/* ezcfg system names */
#define EZCFG_SYS_DEVICE_NAME             "device_name"
#define EZCFG_SYS_SERIAL_NUMBER           "serial_number"
#define EZCFG_SYS_HARDWARE_VERSION        "hardware_version"
#define EZCFG_SYS_SOFTWARE_VERSION        "software_version"

#define EZCFG_SYS_LANGUAGE                "language"
#define EZCFG_SYS_TZ_AREA                 "tz_area"
#define EZCFG_SYS_TZ_LOCATION             "tz_location"

#define EZCFG_SYS_RESTORE_DEFAULTS        "restore_defaults"
#define EZCFG_SYS_UPGRADE_IMAGE           "upgrade_image"
#define EZCFG_SYS_NICS                    "nics"
#define EZCFG_SYS_ELAN_NIC                "elan_nic"
#define EZCFG_SYS_WLAN_NIC                "wlan_nic"
#define EZCFG_SYS_BRLAN_NICS              "brlan_nics"
#define EZCFG_SYS_LAN_NIC                 "lan_nic"
#define EZCFG_SYS_WAN_NIC                 "wan_nic"

#define EZCFG_SYS_DATA_DEV                "data_dev"
#define EZCFG_SYS_MODULES                 "modules"
#define EZCFG_SYS_IPTABLES_MODULES        "iptables_modules"
#define EZCFG_SYS_LD_LIBRARY_PATH         "ld_library_path"

#endif

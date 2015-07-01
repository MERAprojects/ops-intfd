/*
 Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License. You may obtain
 a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 License for the specific language governing permissions and limitations
 under the License.
*/
/************************************************************************//**
 * @defgroup intfd Halon Interface Daemon
 *
 * @brief Halon interface daemon (intfd)
 *
 * The interface daemon manages the Physical Interfaces.
 * Based on the user interface configuration and other parameters which effects
 * the interfaces (like pluggable modules, QSP splittable configuration, etc.),
 * it derives the opearation state of the Interfaces.
 *
 * @{
 *
 * @file
 * Header for intfd daemon
 *
 * @defgroup intfd_public Public Interface
 * The intfd daemon manages the physical interfaces on the OpenHalon platform.
 *
 * @{
 *
 * Public APIs
 *
 * Command line options:
 *
 *     intfd: Halon Interface daemon
 *     usage: intfd [OPTIONS] [DATABASE]
 *     where DATABASE is a socket on which ovsdb-server is listening
 *           (default: "unix:/var/run/openvswitch/db.sock").
 *
 *      Daemon options:
 *        --detach                run in background as daemon
 *        --no-chdir              do not chdir to '/'
 *        --pidfile[=FILE]        create pidfile (default: /var/run/openvswitch/intfd.pid)
 *        --overwrite-pidfile     with --pidfile, start even if already running
 *
 *      Logging options:
 *        -vSPEC, --verbose=SPEC   set logging levels
 *        -v, --verbose            set maximum verbosity level
 *        --log-file[=FILE]        enable logging to specified FILE
 *                                 (default: /var/log/openvswitch/intfd.log)
 *        --syslog-target=HOST:PORT  also send syslog msgs to HOST:PORT via UDP
 *
 *      Other options:
 *        --unixctl=SOCKET        override default control socket name
 *        -h, --help              display this help message
 *
 *
 * Available ovs-apptcl command options are:
 *
 *      coverage/show
 *      exit
 *      list-commands
 *      version
 *      intfd/dump              dumps daemons internal data for debugging.
 *      vlog/disable-rate-limit [module]...
 *      vlog/enable-rate-limit  [module]...
 *      vlog/list
 *      vlog/reopen
 *      vlog/set                {spec | PATTERN:destination:pattern}
 *
 * OVSDB elements usage
 *
 *  The following table rows are READ by intfd:
 *
 *      Interface:name
 *      Interface:pm_info
 *      Interface:user_config
 *      Open_vswitch:cur_cfg
 *
 *  The following columns are WRITTEN by intfd:
 *
 *      Interface:error
 *      Interface:hw_intf_config
 *
 * Linux Files:
 *
 *  The following files are written by intfd:
 *
 *      /var/run/openvswitch/intfd.pid: Process ID for the intfd
 *      /var/run/openvswitch/intfd.<pid>.ctl: Control file for ovs-appctl
 *
 ***************************************************************************/
/** @} end of group intfd_public */

#ifndef __INTFD_H__
#define __INTFD_H__

#include <hc-utils.h>
#include <openvswitch/vlog.h>
#include <openvswitch/compiler.h>

/* Currently only QSFP+ interfaces can be split
 * up into subintfs, and it's always groups of 4. */
#define MAX_SPLIT_COUNT                4

#define STR_EQ(s1, s2)      ((strlen((s1)) == strlen((s2))) && (!strncmp((s1), (s2), strlen((s2)))))

/*********************************************************************
 * /intf._/info [capabilities] flags
 ********************************************************************/
#define SPEED_1G                    1000
#define SPEED_10G                   10000
#define SPEED_40G                   40000

#define PLUGGABLE_FLAG              (uint64_t)0x00000001
#define ENET_1G_CAPABLE_FLAG        (uint64_t)0x00000002
#define ENET_10G_CAPABLE_FLAG       (uint64_t)0x00000004
#define ENET_40G_CAPABLE_FLAG       (uint64_t)0x00000008
#define INTF_SPLIT_4_CAPABLE_FLAG   (uint64_t)0x00000010

/* Following MACROs check for intf's capabilities. */
#define INTF_IS_PLUGGABLE(p)        (0 != ((p)->pm_info.capability_flags & PLUGGABLE_FLAG))
#define INTF_IS_ENET_1G_CAPABLE(p)  (0 != ((p)->pm_info.capability_flags & ENET_1G_CAPABLE_FLAG))
#define INTF_IS_ENET_10G_CAPABLE(p) (0 != ((p)->pm_info.capability_flags & ENET_10G_CAPABLE_FLAG))
#define INTF_IS_ENET_40G_CAPABLE(p) (0 != ((p)->pm_info.capability_flags & ENET_40G_CAPABLE_FLAG))
#define INTF_IS_SPLIT_4_CAPABLE(p)  (0 != ((p)->pm_info.capability_flags & INTF_SPLIT_4_CAPABLE_FLAG))

/*********************************************************************
 * op_connector flags to simplify processing
 *********************************************************************
 * NOTE: Use bit 4  (0x0010) to indicate the connector is supported.
 *       Use bit 5  (0x0020) to indicate general SFP module type.
 *       Use bit 6  (0x0040) to indicate SFP+ 10G speed.
 *       Use bit 7  (0x0080) to indicate general QSFP module type.
 *       Use bit 8  (0x0100) to indicate QSFP 40G speed.
 *       Use bit 9  (0x0200) to indicate QSFP 4 x 10G speed.
 ********************************************************************/

/* First, define the OP_CONNECTOR flags */
#define PM_UNSUPPORTED_FLAG         (uint64_t)0x0000
#define PM_SUPPORTED_FLAG           (uint64_t)0x0010
#define PM_SFP_TYPE_FLAG            (uint64_t)0x0020
#define PM_SFP_10G_FLAG             (uint64_t)0x0040
#define PM_QSFP_PLUS_TYPE_FLAG      (uint64_t)0x0080
#define PM_QSFP_PLUS_40G_FLAG       (uint64_t)0x0100

#define PM_SFP_FLAGS                (PM_SUPPORTED_FLAG | PM_SFP_TYPE_FLAG)

#define PM_SFP_PLUS_FLAGS           (PM_SUPPORTED_FLAG | PM_SFP_TYPE_FLAG | \
                                     PM_SFP_10G_FLAG)

#define PM_QSFP_PLUS_40G_FLAGS      (PM_SUPPORTED_FLAG | PM_QSFP_PLUS_TYPE_FLAG | \
                                     PM_QSFP_PLUS_40G_FLAG)

#define CONNECTOR_SUPPORTED(p)      ((p)->pm_info.op_connector_flags & PM_SUPPORTED_FLAG)
#define CONNECTOR_IS_SFP_FAMILY(p)  ((p)->pm_info.op_connector_flags & PM_SFP_TYPE_FLAG)
#define CONNECTOR_IS_QSFP_FAMILY(p) ((p)->pm_info.op_connector_flags & PM_QSFP_PLUS_TYPE_FLAG)

#define CONNECTOR_IS_SFP(p)         (CONNECTOR_IS_SFP_FAMILY(p) && \
                                     !((p)->pm_info.op_connector_flags & PM_SFP_10G_FLAG))

#define CONNECTOR_IS_SFP_PLUS(p)    (CONNECTOR_IS_SFP_FAMILY(p) && \
                                     ((p)->pm_info.op_connector_flags & PM_SFP_10G_FLAG))

#define CONNECTOR_IS_QSFP_PLUS_40G(p) (CONNECTOR_IS_QSFP_FAMILY(p) && \
                                      ((p)->pm_info.op_connector_flags & PM_QSFP_PLUS_40G_FLAG))

#define CONNECTOR_IS_SFP_RJ45(p) (INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45 == (p)->pm_info.op_connector)
#define CONNECTOR_IS_DAC(p)      (INTERFACE_PM_INFO_CONNECTOR_SFP_DAC == (p)->pm_info.op_connector)
#define CONNECTOR_IS_1G(p)       (CONNECTOR_IS_SFP(p))
#define CONNECTOR_IS_10G(p)      (CONNECTOR_IS_SFP_PLUS(p))
#define CONNECTOR_IS_ABSENT(p)   (INTERFACE_PM_INFO_CONNECTOR_ABSENT == (p)->pm_info.op_connector)

#define CABLE_TECH_IS_ACTIVE(p)  (INTERFACE_PM_INFO_CABLE_TECHNOLOGY_ACTIVE == (p)->pm_info.cable_tech)
#define CABLE_TECH_IS_PASSIVE(p) (INTERFACE_PM_INFO_CABLE_TECHNOLOGY_PASSIVE == (p)->pm_info.cable_tech)

extern void intfd_ovsdb_init(const char *db_path);
extern void intfd_ovsdb_exit(void);
extern void intfd_run(void);
extern void intfd_wait(void);
extern void intfd_debug_dump(struct ds *ds, int argc, const char *argv[]);

#endif /* __INTFD_H__ */
/** @} end of group intfd */
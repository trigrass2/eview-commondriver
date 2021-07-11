cmake_minimum_required(VERSION 2.6)

PROJECT (bacnetdrv)

ADD_DEFINITIONS(-D_XKEYCHECK_H)

#Setting Source Files
SET(SRCS ${SRCS} main.cpp AutoGroup_ObjDev.cpp
stack/abort.cpp stack/address.cpp  stack/alarm_ack.cpp
stack/apdu.cpp stack/arf.cpp  stack/awf.cpp
stack/bacaddr.cpp  stack/bacapp.cpp  stack/bacdcode.cpp
stack/bacdevobjpropref.cpp stack/bacerror.cpp stack/bacint.cpp
stack/bacprop.cpp stack/bacpropstates.cpp stack/bacreal.cpp
stack/bacstr.cpp stack/bactext.cpp stack/bigend.cpp
stack/bip.cpp stack/bvlc.cpp stack/cov.cpp
stack/crc.cpp stack/datalink.cpp stack/datetime.cpp
stack/dcc.cpp stack/debug.cpp stack/event.cpp
stack/fifo.cpp stack/filename.cpp stack/getevent.cpp
stack/get_alarm_sum.cpp stack/iam.cpp stack/ihave.cpp
stack/indtext.cpp stack/key.cpp stack/keylist.cpp
stack/lso.cpp stack/memcopy.cpp
stack/mstp.cpp stack/mstptext.cpp stack/npdu.cpp
stack/proplist.cpp stack/ptransfer.cpp stack/rd.cpp
stack/readrange.cpp stack/reject.cpp stack/ringbuf.cpp
stack/rp.cpp stack/rpm.cpp stack/sbuf.cpp
stack/timestamp.cpp stack/timesync.cpp stack/tsm.cpp
stack/version.cpp stack/whohas.cpp stack/whois.cpp
stack/wp.cpp stack/wpm.cpp 

object/ai.cpp object/ao.cpp object/av.cpp object/bacfile.cpp object/ object/bi.cpp
object/bo.cpp object/bv.cpp object/csv.cpp object/ object/device.cpp object/lc.cpp
object/lo.cpp object/lsp.cpp object/ object/ms-input.cpp object/mso.cpp object/ object/msv.cpp
object/trendlog.cpp


handler/dlenv.cpp             handler/h_alarm_ack.cpp       handler/h_arf.cpp
handler/h_arf_a.cpp           handler/h_awf.cpp             handler/h_ccov.cpp
handler/h_cov.cpp             handler/h_dcc.cpp             handler/h_gas_a.cpp
handler/h_getevent.cpp        handler/h_get_alarm_sum.cpp   handler/h_iam.cpp
handler/h_ihave.cpp           handler/h_lso.cpp             handler/h_npdu.cpp
handler/h_pt.cpp              handler/h_pt_a.cpp            handler/h_rd.cpp
    handler/h_rp.cpp              handler/h_rpm.cpp
handler/h_rpm_a.cpp           handler/h_rp_a.cpp            handler/h_rr.cpp
handler/h_rr_a.cpp            handler/h_ts.cpp              handler/h_ucov.cpp
handler/h_upt.cpp             handler/h_whohas.cpp          handler/h_whois.cpp
handler/h_wp.cpp              handler/h_wpm.cpp             handler/noserv.cpp
handler/objects.cpp           handler/s_ack_alarm.cpp       handler/s_arfs.cpp
handler/s_awfs.cpp            handler/s_cevent.cpp          handler/s_cov.cpp
handler/s_dcc.cpp             handler/s_iam.cpp             handler/s_ihave.cpp
handler/s_lso.cpp             handler/s_ptransfer.cpp       handler/s_rd.cpp
handler/s_readrange.cpp       handler/s_router.cpp          handler/s_rp.cpp
handler/s_rpm.cpp             handler/s_ts.cpp              handler/s_uevent.cpp
handler/s_upt.cpp             handler/s_whohas.cpp          handler/s_whois.cpp
handler/s_wp.cpp              handler/s_wpm.cpp             handler/txbuf.cpp
)

IF(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    SET(SRCS ${SRCS} ports/win32/bip-init.cpp)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    SET(SRCS ${SRCS} ports/linux/bip-init.cpp)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES Windows)

aux_source_directory(. bacnet-stack/src)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME bacnetdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

IF(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    SET(LINK_LIBS pkdrvcmn ws2_32 iphlpapi.lib pkcomm)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    SET(LINK_LIBS pkdrvcmn pkcomm)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES Windows)

INCLUDE_DIRECTORIES(../../drivers/bacnetdrv/include)
IF(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    INCLUDE_DIRECTORIES(../../drivers/bacnetdrv/ports/win32)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    INCLUDE_DIRECTORIES(../../drivers/bacnetdrv/ports/linux)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES Windows)


SET(SPECOUTDIR /drivers/bacnetdrv)
INCLUDE(../../CMakeCommonLib)
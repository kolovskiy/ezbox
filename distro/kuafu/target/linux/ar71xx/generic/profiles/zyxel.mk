#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/NBG_460N_550N_550NH
	NAME:=Zyxel NBG 460N/550N/550NH
	PACKAGES:=kmod-ath9k kmod-rtc-pcf8563 swconfig uboot-ar71xx-nbg460n_550n_550nh
endef

define Profile/NBG_460N_550N_550NH/Description
	Package set optimized for the Zyxel NBG 460N/550N/550NH Routers.
endef

$(eval $(call Profile,NBG_460N_550N_550NH))


include mf_os_detect

######################
# Compiler && output #
######################

compiler = g++
includeDirs = -I../mlib

cpp_options = -Wall -std=c++11 $(includeDirs) -c
c_options = -Wall $(includeDirs) -c

objdir = bin

# binaries
srvBin = i04Dns

# linker options
ifeq ($(detected_OS),WINDOWS)
srvLnk = -lIphlpapi -lws2_32 
else
srvLnk = -lpthread
endif

######################
#     Build Rules    #
######################

# Default rule
debug: debug-init $(srvBin)

release: release-init $(srvBin)

######################
#      Objects       #
######################

src_system = ../mlib/system.c
obj_system = $(objdir)/base_system.o
rul_system = $(obj_system): $(shell makedep $(src_system))
$(rul_system)
	@echo $(obj_system)
	@$(compiler) $(c_options) -o $@ $<

src_heapDbg = src/base/heap_dbg.c
obj_heapDbg = $(objdir)/base_heap_dbg.o
rul_heapDbg = $(obj_heapDbg): $(shell makedep $(src_heapDbg))
$(rul_heapDbg)
	@echo $(obj_heapDbg)
	@$(compiler) $(c_options) -o $@ $<

src_util = src/util/util.cpp
obj_util = $(objdir)/util_util.o
rul_util = $(obj_util): $(shell makedep $(src_util))
$(rul_util)
	@echo $(obj_util)
	@$(compiler) $(cpp_options) -o $@ $<

src_enumip = ../mlib/enumip.cpp
obj_enumip = $(objdir)/base_enumip.o
rul_enumip = $(obj_enumip): $(shell makedep $(src_enumip))
# $@ matches the target; $< matches the first dependent
$(rul_enumip)
	@echo $(obj_enumip)
	@$(compiler) $(cpp_options) -o $@ $<

src_logger = src/server/logger.cpp
obj_logger = $(objdir)/util_logger.o
rul_logger = $(obj_logger): $(shell makedep $(src_logger))
$(rul_logger)
	@echo $(obj_logger)
	@$(compiler) $(cpp_options) -o $@ $<

src_nameFltr = src/server/dns/CNameFilter.cpp
obj_nameFltr = $(objdir)/srv_dns_CNameFilter.o
rul_nameFltr = $(obj_nameFltr): $(shell makedep $(src_nameFltr))
$(rul_nameFltr)
	@echo $(obj_nameFltr)
	@$(compiler) $(cpp_options) -o $@ $<

src_localRec = src/server/dns/CLocalRecord.cpp
obj_localRec = $(objdir)/srv_dns_CLocalRecord.o
rul_localRec = $(obj_localRec): $(shell makedep $(src_localRec))
$(rul_localRec)
	@echo $(obj_localRec)
	@$(compiler) $(cpp_options) -o $@ $<

src_bInStream = src/util/CBufferInputStream.cpp
obj_bInStream = $(objdir)/util_BufferInputStream.o
rul_bInStream = $(obj_bInStream): $(shell makedep $(src_bInStream))
$(rul_bInStream)
	@echo $(obj_bInStream)
	@$(compiler) $(cpp_options) -o $@ $<

src_bOutStream = src/util/CBufferOutputStream.cpp
obj_bOutStream = $(objdir)/util_BufferOutputStream.o
rul_bOutStream = $(obj_bOutStream): $(shell makedep $(src_bOutStream))
$(rul_bOutStream)
	@echo $(obj_bOutStream)
	@$(compiler) $(cpp_options) -o $@ $<

src_2ndSrv = src/server/dns/CSecondaryServer.cpp
obj_2ndSrv = $(objdir)/srv_dns_CSecondaryServer.o
rul_2ndSrv = $(obj_2ndSrv): $(shell makedep $(src_2ndSrv))
$(rul_2ndSrv)
	@echo $(obj_2ndSrv)
	@$(compiler) $(cpp_options) -o $@ $<

src_confFile = src/server/CConfigFile.cpp
obj_confFile = $(objdir)/srv_ConfigFile.o
rul_confFile = $(obj_confFile): $(shell makedep $(src_confFile))
$(rul_confFile)
	@echo $(obj_confFile)
	@$(compiler) $(cpp_options) -o $@ $<

src_dnsSrv = src/server/dns/dnsServer.cpp
obj_dnsSrv = $(objdir)/srv_dns_server.o
rul_dnsSrv = $(obj_dnsSrv): $(shell makedep $(src_dnsSrv))
$(rul_dnsSrv)
	@echo $(obj_dnsSrv)
	@$(compiler) $(cpp_options) -o $@ $<

src_srvMain = src/server/main.cpp
obj_srvMain = $(objdir)/srv_main.o
rul_srvMain = $(obj_srvMain): $(shell makedep $(src_srvMain))
$(rul_srvMain)
	@echo $(obj_srvMain)
	@$(compiler) $(cpp_options) -o $@ $<

ifeq ($(OS),Windows_NT)
src_srv_res = res/srv.rc
obj_srv_res = $(objdir)/srv.res
endif

######################
#      Porjects      #
######################

# DNS Server
srvObjects = $(obj_heapDbg) $(obj_system) $(obj_util) $(obj_enumip) $(obj_logger) $(obj_nameFltr) $(obj_localRec) $(obj_bInStream) $(obj_bOutStream) $(obj_2ndSrv) $(obj_confFile) $(obj_dnsSrv) $(obj_srvMain)

ifeq ($(OS),Windows_NT)
srvObjects += $(obj_srv_res)
endif

$(srvBin): $(srvObjects)
	@echo Linking $(srvBin)
	@$(compiler) -Wall -o $(srvBin) $(srvObjects) $(srvLnk) -static-libgcc -static-libstdc++
	@echo Done...

######################
#     Main rules     #
######################

ifeq ($(detected_OS),WINDOWS)
$(obj_srv_res):
	@echo compiling resources
	@windres $(src_srv_res) -O coff -o $(obj_srv_res)

endif

# Init Rule
init:
ifeq ($(detected_OS),WINDOWS)
	@if not exist $(objdir) mkdir $(objdir)
	@cls
else
	@mkdir -p $(objdir)
	@clear
endif

debug-init: init
	@echo building debug
	$(eval cpp_options += -g -D DEBUG_CONF)
	$(eval c_options += -g -D DEBUG_CONF)

release-init: init
	@echo building release
	$(eval cpp_options += -g -D RELEASE_CONF)
	$(eval c_options += -g -D RELEASE_CONF)

# Clean Rule
clean:
ifeq ($(detected_OS),WINDOWS)
	@del /q $(objdir)\*
	@for /d %%x in (bin\*) do @rd /s /q "%%x"
	@del /f /q *.exe
else
	@rm -r $(objdir)
	@rm -r $(srvBin)
endif

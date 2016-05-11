
CXX= clang++

CFLAG= 
COPT= -O2 -fomit-frame-pointer -std=c++11
CWARN= -Wall -Wextra -Wredundant-decls
CWARNIGNORE= -Wno-unused-result -Wno-strict-aliasing
CINCLUDE=
CDEF=

#ifdef debug
CFLAG+= -O0 -g -Wno-format -fno-omit-frame-pointer
BUILDTYPE= debug
#else
#CFLAG+= -DNDEBUG
#BUILDTYPE= release
#endif

# temp
CDEF+= -DEQP_DISABLE_PACKET_COMPRESSION=1

DIRBIN= bin/

##############################################################################
# Common
##############################################################################
DIRCOMMON= src/common/
BCOMMON= build/$(BUILDTYPE)/common/
_OCOMMON= \
 file.o   exception.o   aligned.o   crc.o
_HCOMMON= define.hpp terminal.hpp bit.hpp \
 file.hpp exception.hpp aligned.hpp crc.hpp
OCOMMON= $(patsubst %,$(BCOMMON)%,$(_OCOMMON))
HCOMMON= $(patsubst %,$(DIRCOMMON)%,$(_HCOMMON))

CINCLUDE+= -I$(DIRCOMMON)
OCOMMON_ALL= $(OCOMMON)
HCOMMON_ALL= $(HCOMMON)

##############################################################################
# Common / DB
##############################################################################
DIRCOMMON_DB= $(DIRCOMMON)db/
BCOMMON_DB= $(BCOMMON)db/
_OCOMMON_DB= database.o
_HCOMMON_DB= database.hpp
OCOMMON_DB= $(patsubst %,$(BCOMMON_DB)%,$(_OCOMMON_DB))
HCOMMON_DB= $(patsubst %,$(DIRCOMMON_DB)%,$(_HCOMMON_DB))

CINCLUDE+= -I$(DIRCOMMON_DB)
OCOMMON_ALL+= $(OCOMMON_DB)
HCOMMON_ALL+= $(HCOMMON_DB)

##############################################################################
# Common / Log
##############################################################################
DIRCOMMON_LOG= $(DIRCOMMON)log/
BCOMMON_LOG= $(BCOMMON)log/
_OCOMMON_LOG= log_writer_common.o   log_writer.o
_HCOMMON_LOG= log_writer_common.hpp log_writer.hpp
OCOMMON_LOG= $(patsubst %,$(BCOMMON_LOG)%,$(_OCOMMON_LOG))
HCOMMON_LOG= $(patsubst %,$(DIRCOMMON_LOG)%,$(_HCOMMON_LOG))

CINCLUDE+= -I$(DIRCOMMON_LOG)
OCOMMON_ALL+= $(OCOMMON_LOG)
HCOMMON_ALL+= $(HCOMMON_LOG)

##############################################################################
# Common / Net
##############################################################################
DIRCOMMON_NET= $(DIRCOMMON)net/
BCOMMON_NET= $(BCOMMON)net/
_OCOMMON_NET= \
 udp_socket.o   protocol_handler.o   packet_tracker.o   ack_manager.o
_HCOMMON_NET= eq_packet_protocol.hpp packet_structs_protocol.hpp \
 packet_structs_login.hpp opcodes.hpp opcodes_canonical.hpp opcodes_titanium.hpp \
 opcodes_sof.hpp opcodes_sod.hpp opcodes_underfoot.hpp opcodes_rof.hpp \
 opcodes_rof2.hpp opcode_translation.hpp \
 udp_socket.hpp protocol_handler.hpp packet_tracker.hpp ack_manager.hpp
OCOMMON_NET= $(patsubst %,$(BCOMMON_NET)%,$(_OCOMMON_NET))
HCOMMON_NET= $(patsubst %,$(DIRCOMMON_NET)%,$(_HCOMMON_NET))

CINCLUDE+= -I$(DIRCOMMON_NET)
OCOMMON_ALL+= $(OCOMMON_NET)
HCOMMON_ALL+= $(HCOMMON_NET)

##############################################################################
# Common / ShareMem
##############################################################################
DIRCOMMON_SHAREMEM= $(DIRCOMMON)shm/
BCOMMON_SHAREMEM= $(BCOMMON)shm/
_OCOMMON_SHAREMEM= share_mem.o
_HCOMMON_SHAREMEM= define_share_mem.hpp share_mem.hpp
OCOMMON_SHAREMEM= $(patsubst %,$(BCOMMON_SHAREMEM)%,$(_OCOMMON_SHAREMEM))
HCOMMON_SHAREMEM= $(patsubst %,$(DIRCOMMON_SHAREMEM)%,$(_HCOMMON_SHAREMEM))

CINCLUDE+= -I$(DIRCOMMON_SHAREMEM)
OCOMMON_ALL+= $(OCOMMON_SHAREMEM)
HCOMMON_ALL+= $(HCOMMON_SHAREMEM)

##############################################################################
# Common / Sync
##############################################################################
DIRCOMMON_SYNC= $(DIRCOMMON)sync/
BCOMMON_SYNC= $(BCOMMON)sync/
_OCOMMON_SYNC= \
 atomic_mutex.o   semaphore.o   ring_buffer.o   ipc_buffer.o
_HCOMMON_SYNC= server_op.hpp source_id.hpp master_semaphore.hpp \
 atomic_mutex.hpp semaphore.hpp ring_buffer.hpp ipc_buffer.hpp
OCOMMON_SYNC= $(patsubst %,$(BCOMMON_SYNC)%,$(_OCOMMON_SYNC))
HCOMMON_SYNC= $(patsubst %,$(DIRCOMMON_SYNC)%,$(_HCOMMON_SYNC))

CINCLUDE+= -I$(DIRCOMMON_SYNC)
OCOMMON_ALL+= $(OCOMMON_SYNC)
HCOMMON_ALL+= $(HCOMMON_SYNC)

##############################################################################
# Common / Time
##############################################################################
DIRCOMMON_TIME= $(DIRCOMMON)time/
BCOMMON_TIME= $(BCOMMON)time/
_OCOMMON_TIME= clock.o timer_pool.o timer.o
_HCOMMON_TIME= clock.hpp timer_pool.hpp timer.hpp
OCOMMON_TIME= $(patsubst %,$(BCOMMON_TIME)%,$(_OCOMMON_TIME))
HCOMMON_TIME= $(patsubst %,$(DIRCOMMON_TIME)%,$(_HCOMMON_TIME))

CINCLUDE+= -I$(DIRCOMMON_TIME)
OCOMMON_ALL+= $(OCOMMON_TIME)
HCOMMON_ALL+= $(HCOMMON_TIME)

##############################################################################
# Master
##############################################################################
DIRMASTER= src/master/
BMASTER= build/$(BUILDTYPE)/master/
_OMASTER= master_main.o \
 master.o   log_writer_master.o   ipc_master.o
_HMASTER= \
 master.hpp log_writer_master.hpp ipc_master.hpp
OMASTER= $(patsubst %,$(BMASTER)%,$(_OMASTER))
HMASTER= $(patsubst %,$(DIRMASTER)%,$(_HMASTER))

INCLUDEMASTER= -I$(DIRMASTER)
BINMASTER= $(DIRBIN)eqp-master

##############################################################################
# Login
##############################################################################
DIRLOGIN= src/login/
BLOGIN= build/$(BUILDTYPE)/login/
_OLOGIN= login_main.o \
 login.o   login_crypto.o
_HLOGIN= \
 login.hpp login_crypto.hpp
OLOGIN= $(patsubst %,$(BLOGIN)%,$(_OLOGIN))
HLOGIN= $(patsubst %,$(DIRLOGIN)%,$(_HLOGIN))

INCLUDELOGIN= -I$(DIRLOGIN)
BINLOGIN= $(DIRBIN)eqp-login

##############################################################################
# Char Select
##############################################################################
DIRCHARSELECT= src/char_select/
BCHARSELECT= build/$(BUILDTYPE)/char_select/
_OCHARSELECT= char_select_main.o \
 char_select.o   char_select_client.o   char_select_socket.o
_HCHARSELECT= \
 char_select.hpp char_select_client.hpp char_select_socket.hpp
OCHARSELECT= $(patsubst %,$(BCHARSELECT)%,$(_OCHARSELECT))
HCHARSELECT= $(patsubst %,$(DIRCHARSELECT)%,$(_HCHARSELECT))

INCLUDECHARSELECT= -I$(DIRCHARSELECT)
BINCHARSELECT= $(DIRBIN)eqp-char-select

##############################################################################
# Core Linker flags
##############################################################################
LFLAGS=
LSTATIC= 
LDYNAMIC= -lm -pthread -lrt -lsqlite3 -lz

##############################################################################
# Util
##############################################################################
Q= @
E= @echo
RM= rm -f 
AR= ar -cfr

##############################################################################
# Build rules
##############################################################################
.PHONY: default all clean

default all: master login char-select

master: $(BINMASTER)

login: $(BINLOGIN)

char-select: $(BINCHARSELECT)

amalg: amalg-master amalg-login amalg-char-select

amalg-master:
	$(Q)luajit amalg/amalg.lua master src/master/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINMASTER)"
	$(Q)$(CXX) -o $(BINMASTER) amalg/amalg_master.cpp $(LSTATIC) $(LDYNAMIC) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMASTER)

amalg-login:
	$(Q)luajit amalg/amalg.lua login src/login/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINLOGIN)"
	$(Q)$(CXX) -o $(BINLOGIN) amalg/amalg_login.cpp $(LSTATIC) $(LDYNAMIC) -lcrypto $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGIN)

amalg-char-select:
	$(Q)luajit amalg/amalg.lua char_select src/char_select/
	$(E) "\033[0;32mCreating amalgamated source file\033[0m"
	$(E) "Building $(BINCHARSELECT)"
	$(Q)$(CXX) -o $(BINCHARSELECT) amalg/amalg_char_select.cpp $(LSTATIC) $(LDYNAMIC) $(LFLAGS) $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECHARSELECT)

$(BINMASTER): $(OMASTER) $(OCOMMON_ALL)
	$(E) "Linking $@"
	$(Q)$(CXX) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

$(BINLOGIN): $(OLOGIN) $(OCOMMON_ALL)
	$(E) "Linking $@"
	$(Q)$(CXX) -o $@ $^ $(LSTATIC) $(LDYNAMIC) -lcrypto $(LFLAGS)

$(BINCHARSELECT): $(OCHARSELECT) $(OCOMMON_ALL)
	$(E) "Linking $@"
	$(Q)$(CXX) -o $@ $^ $(LSTATIC) $(LDYNAMIC) $(LFLAGS)

$(BCOMMON)%.o: $(DIRCOMMON)%.cpp $(HCOMMON_ALL)
	$(E) "\033[0;32mCXX       $@\033[0m"
	$(Q)$(CXX) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE)

$(BMASTER)%.o: $(DIRMASTER)%.cpp $(HMASTER)
	$(E) "\033[0;32mCXX       $@\033[0m"
	$(Q)$(CXX) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDEMASTER)

$(BLOGIN)%.o: $(DIRLOGIN)%.cpp $(HLOGIN)
	$(E) "\033[0;32mCXX       $@\033[0m"
	$(Q)$(CXX) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDELOGIN)

$(BCHARSELECT)%.o: $(DIRCHARSELECT)%.cpp $(HCHARSELECT)
	$(E) "\033[0;32mCXX       $@\033[0m"
	$(Q)$(CXX) -c -o $@ $< $(COPT) $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE) $(INCLUDECHARSELECT)

##############################################################################
# Clean rules
##############################################################################
clean-common:
	$(Q)$(RM) $(BCOMMON)*.o
	$(Q)$(RM) $(BCOMMON_DB)*.o
	$(Q)$(RM) $(BCOMMON_LOG)*.o
	$(Q)$(RM) $(BCOMMON_SHAREMEM)*.o
	$(Q)$(RM) $(BCOMMON_SYNC)*.o
	$(Q)$(RM) $(BCOMMON_TIME)*.o
	$(E) "Cleaned common"

clean-master:
	$(Q)$(RM) $(BMASTER)*.o
	$(Q)$(RM) $(BINMASTER)
	$(E) "Cleaned master"

clean-login:
	$(Q)$(RM) $(BLOGIN)*.o
	$(Q)$(RM) $(BINLOGIN)
	$(E) "Cleaned login"

clean-char-select:
	$(Q)$(RM) $(BCHARSELECT)*.o
	$(Q)$(RM) $(BINCHARSELECT)
	$(E) "Cleaned char-select"

clean: clean-common clean-master clean-login clean-char-select


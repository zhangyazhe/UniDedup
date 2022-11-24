bin_PROGRAMS=agent client
agent_SOURCES=Agent.cc
agent_LDADD=src/common/libcommon.a src/protocol/libprotocol.a src/util/libutil.a
client_SOURCES=Client.cc
client_LDADD=src/common/libcommon.a src/protocol/libprotocol.a src/util/libutil.a
SUBDIRS=src/common src/protocol src/util
LIBS=-lpthread -lcrypto -lglib -lredis++ -lhiredis
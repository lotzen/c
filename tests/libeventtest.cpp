
#include "gtest.h"

namespace Test {

#include "../libpubnub/pubnub.h"
#include "../libpubnub/pubnub-priv.h"

#undef PUBNUB_API
#define PUBNUB_API


struct event {
	int unused;
};

#define EV_READ 1
#define EV_WRITE 2
#define EV_PERSIST 4
#define evtimer_new(b, cb, arg)	event_new((b), -1, 0, (cb), (arg))

struct pubnub_libevent;

class LibEventTest : public ::testing::Test
{
public:
	pubnub *p;
	static struct pubnub_libevent *libevent;
	static int addEventCnt;
	static int delEventCnt;
	static bool isPendingCalled;
	virtual void SetUp();
	virtual void TearDown();
};

struct pubnub_libevent *LibEventTest::libevent;
int LibEventTest::addEventCnt;
int LibEventTest::delEventCnt;
bool LibEventTest::isPendingCalled;

int event_add(struct event *, const struct timeval *)
{
	LibEventTest::addEventCnt++;
	return 0;
}

int evtimer_add(struct event *, const struct timeval *)
{
	return 0;
}

int event_del(struct event *)
{
	LibEventTest::delEventCnt++;
	return 0;
}

int evtimer_del(struct event *)
{
	return 0;
}

void event_set(event *, int, short, void(*)(int, short, void *), void *)
{
}

void evtimer_set(void*, void(*)(int, short, void *), void*)
{
}

int	evtimer_pending(struct event *, struct timeval *)
{
	LibEventTest::isPendingCalled = true;
	return 1;
}

struct event *event_new(struct event_base *, int, short, void (*)(int, short int, void*), void *)
{
	return (struct event *)malloc(sizeof(struct event));
}

void event_free(struct event *ev)
{
	LibEventTest::delEventCnt++;
	free(ev);
}

#include "../libpubnub/pubnub-libevent.c"


void LibEventTest::SetUp() {
	libevent = pubnub_libevent_init(NULL);
	p = pubnub_init("demo", "demo", &pubnub_libevent_callbacks, libevent);
	addEventCnt = 0;
	delEventCnt = 0;
	isPendingCalled = false;
}
void LibEventTest::TearDown() {
	pubnub_done(p);
}

TEST_F(LibEventTest, StopWait) {
	pubnub_libevent_stop_wait(p, p->cb_data);
	ASSERT_TRUE(isPendingCalled);
}

TEST_F(LibEventTest, AddDelSockets) {
	pubnub_libevent_add_socket(p, p->cb_data, 10, 1, NULL, NULL);
	pubnub_libevent_add_socket(p, p->cb_data, 10, 2, NULL, NULL);
	pubnub_libevent_rem_socket(p, p->cb_data, 10);
	ASSERT_EQ(2, addEventCnt);
	ASSERT_EQ(1, delEventCnt);
}

TEST_F(LibEventTest, TryDelWrongSocket) {
	pubnub_libevent_add_socket(p, p->cb_data, 10, 1, NULL, NULL);
	pubnub_libevent_rem_socket(p, p->cb_data, 11);
	ASSERT_EQ(1, addEventCnt);
	ASSERT_EQ(0, delEventCnt);
}


}

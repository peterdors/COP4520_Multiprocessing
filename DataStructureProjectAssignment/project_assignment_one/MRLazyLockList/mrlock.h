#include <atomic>
#include <bitset>
using namespace std;

#define NUM_BITS (int) 1e6

struct cell
{
	atomic<uint32_t> seq;
	bitset<NUM_BITS> bits;
};

struct mrlock
{
	cell* buffer;
	uint32_t mask;
	atomic<uint32_t> head;
	atomic<uint32_t> tail;
};

void init(mrlock& l, uint32_t siz)
{
	l.buffer = new cell[siz];
	l.mask = siz -1;
	l.head.store(0, memory_order_relaxed);
	l.tail.store(0, memory_order_relaxed);

	for (uint32_t i=0; i<siz; i++)
	{
		(l.buffer[i]).bits.set();
		(l.buffer[i]).seq.store(i, memory_order_relaxed);
	}
}

void uninit(mrlock& l)
{
	delete[] l.buffer;
}

uint32_t lock(mrlock& l, bitset<NUM_BITS> r)
{
	cell* c;
	uint32_t pos;
	for (;;)
	{
		pos = l.tail.load(memory_order_relaxed);
		c = &l.buffer[pos & l.mask];
		uint32_t seq = c->seq.load(memory_order_acquire);
		int32_t dif = (int32_t)seq - (int32_t)pos;
		if(dif == 0)
		{
			if (l.tail.compare_exchange_weak(pos, pos+1, memory_order_relaxed))
				break;
		}
	}
	c->bits = r;
	c->seq.store(pos+1, memory_order_release);
	uint32_t spin = l.head;
	while (spin != pos)
	{

		void* cst1 = reinterpret_cast<void*>(&l.buffer[spin & l.mask].bits);
		void* cst2 = reinterpret_cast<void*>(&r);
		if (
			(pos - l.buffer[spin & l.mask].seq) > l.mask ||
			// !((uintptr_t)cst1 & (uintptr_t)cst2)
			!(l.buffer[spin & l.mask].bits == r)
		)
			spin++;
	}
	return pos;
}

void unlock(mrlock& l, uint32_t h)
{
	l.buffer[h & l.mask].bits.reset();
	uint32_t pos = l.head.load(memory_order_relaxed);
	while (l.buffer[pos & l.mask].bits == 0)
	{
		cell* c = &l.buffer[pos & l.mask];
		uint32_t seq = c->seq.load(memory_order_acquire);
		int32_t dif = (int32_t)seq - (int32_t)(pos+1);

		if (dif == 0)
		{
			if (l.head.compare_exchange_weak(pos, pos+1, memory_order_relaxed))
			{
				c->bits.set();
				c->seq.store(pos + l.mask + 1, memory_order_release);
			}
		}

		pos = l.head.load(memory_order_relaxed);
	}
}

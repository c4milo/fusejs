#include <atomic>
#define MPSC_QUEUE_EOF 1
#define MPSC_QUEUE_FULL 2

/* 
 
This is a very simple MPSC queue using a ring_buffer.
This is meant to be used in conjunction with fusejs

tail points to the object before 
head points to the object after
*/

template <typename T>
class mpsc_queue_t
{
public:
	mpsc_queue_t(uint ring_size) {
		data = (T*) malloc(ring_size * sizeof(T));
		if(data == nullptr){
			//TODO: thow error about not enough memory;
			printf("WE WERE NOT ABLE TO ALLOCATE MEMOMRY FOR THE QUEUE! \n");
			return;
		}

		claimed = 0;
		next_to_be_claimed = 1;
		head = 1;
		this->ring_size = ring_size;
		ring_mask = ring_size - 1;

	}
	uint consume(T **value){
		/* 
		This function returns the next item to be consumed
		*/
		uint _head = head;
		if(!( _head == (tail+1) || (tail==ring_mask && _head==0) )){
			tail = (tail + 1 ) & ring_mask;
			*value = &(data[tail]);
			// uint _head = head;
			// uint _claimed = claimed;
			// uint _next = next_to_be_claimed;
			// printf("consumed tail %u -- claimed %u -- head %u -- next %u\n", 
			//	tail, _claimed, _head,_next);
			return 0;
		}
		return MPSC_QUEUE_EOF;
	}

	uint producer_claim_next(T **value){
		uint idx = next_to_be_claimed++;
		next_to_be_claimed.fetch_and(ring_mask); 
		if(idx != tail){
			claimed++;	
			*value = &(data[idx]);
			// uint _head = head;
			// uint _claimed = claimed;
			// uint _next = next_to_be_claimed;
			// printf("claimed idx %lu -- claimed %lu -- head %u -- next %u\n", 
			// 	idx, _claimed, _head, _next);

			return 0;
		}
		return MPSC_QUEUE_FULL;
	}
	uint producer_publish(){
		uint _claimed = --claimed;
		uint _head = head;		
		_head = _head & ring_mask;
		uint idx;
		if( _claimed == 0){
			idx = next_to_be_claimed;
			head = idx & ring_mask ;

			// if empty, wake up consumer thread;
			if( ( _head == (tail+1) || (tail==ring_mask && _head==0) )){
				uv_async_send(&uv_async_handle);
			}

		}

		// _head = head;
		// uint _next = next_to_be_claimed;
		// printf("publish claimed %u -- head %u -- next %u\n", 
		// 		_claimed, _head, _next);

		return 0;

	}
private:
	T *data;
	std::atomic<uint> claimed;
	std::atomic<uint> next_to_be_claimed;
	std::atomic<uint> head; 
	uint tail;
	uint ring_size;
	uint ring_mask;
	uint mask; 
	mpsc_queue_t(const mpsc_queue_t&) = delete;
	void operator=(const mpsc_queue_t&) = delete;
};
#include <atomic>
#define MPSC_QUEUE_EOF 1
#define MPSC_QUEUE_FULL 2

/* 
 
This is a very simple MPSC queue using a ring_buffer.
This is meant to be used in conjunction with fusejs

head points to the object before 
tail points to the object after
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
		tail = 1;
		this->ring_size = ring_size;
		ring_mask = ring_size - 1;

	}
	uint consume(T **value){
		/* 
		This function returns the next item to be consumed
		*/
		if( tail != (head+1) ){
			*value = &(data[++head]);
			// uint _tail = tail;
			// uint _claimed = claimed;
			// printf("consumed head %lu -- claimed %lu -- tail %lu\n", 
			// 	head, _claimed, _tail);
			return 0;
		}
		return MPSC_QUEUE_EOF;
	}

	uint producer_claim_next(T **value){
		uint idx = next_to_be_claimed++;
		idx = idx & ring_mask; //this is quicker than doing a module if the ring_size is a power of 2
		if(idx != head){
			++claimed;
			*value = &(data[idx]);
			// uint _tail = tail;
			// uint _claimed = claimed;
			// printf("claimed idx %lu -- claimed %lu -- tail %lu\n", 
			// 	idx, _claimed, _tail);

			return 0;
		}
		return MPSC_QUEUE_FULL;
	}
	uint producer_publish(){
		if( --claimed == 0){
			int idx = next_to_be_claimed;
			tail = idx;
		}
		return 0;

	}
private:
	T *data;
	std::atomic<uint> claimed;
	std::atomic<uint> next_to_be_claimed;
	std::atomic<uint> tail; 
	uint head;
	uint ring_size;
	uint ring_mask;
	uint mask; 

};
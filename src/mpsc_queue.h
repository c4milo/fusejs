#define MPSC_QUEUE_EOF -1
#define MPSC_QUEUE_FULL -2

#include <atomic>
#include <thread>

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
		tail = 0;
		this->ring_size = ring_size;
		ring_mask = ring_size - 1;

	}
	~mpsc_queue_t(){
		free(data);
	}

	uint consume(T **value){
		/* 
		This function returns the next item to be consumed
		*/
		volatile uint _tail = (tail + 1) & ring_mask;

		volatile uint _head = head;
		if( _head != _tail  ){
			tail = ( tail + 1) & ring_mask; 
			*value = &(data[_tail]);
			// volatile uint _head = head;
			// volatile uint _claimed = claimed;
			// volatile uint _next = next_to_be_claimed;
			// printf("consumed tail %u -- claimed %u -- head %u -- next %u-- thread id %u\n", 
			// 	_tail, _claimed, _head,_next,std::this_thread::get_id());
			return 0;
		}
		return MPSC_QUEUE_EOF;
	}

	int64_t producer_claim_next(T **value){

		volatile uint _tail;
		uint idx;
		// idx_ptr = (uint) &idx;


		do
		{
		    _tail =  tail;
		    idx = next_to_be_claimed;
		    _tail = _tail & ring_mask;
		    idx = idx & ring_mask;
		    if ( ( (_tail) & ring_mask ) ==
		        ( (idx+1) & ring_mask ) )
		    {
		        // the queue is full
		        uv_async_send(&uv_async_handle);
		        std::this_thread::yield();
		        return producer_claim_next(value);
		    }

		} while ( !std::atomic_compare_exchange_strong(&next_to_be_claimed, &idx, (idx+1) & ring_mask ));
		//!next_to_be_claimed.compare_exchange_strong(  (uint) idx, (uint)(idx + 1) ,std::memory_order_seq_cst,std::memory_order_seq_cst) );
		// if( tail == idx){
		// 	do{
		// 		printf("queue full idx %u\tthread_id %u\n", idx, std::this_thread::get_id());
		// 		uv_async_send(&uv_async_handle);
		// 		std::this_thread::yield();
		// 		idx = next_to_be_claimed;
		// 	} while (idx == tail);
		// 	idx = next_to_be_claimed++;
		// }

		// next_to_be_claimed.fetch_and(ring_mask); 

		volatile uint _head = head;
		uint _claimed =  claimed.fetch_add(1) + 1;
		// if( (idx < _head) && (idx > tail)){
		// 	return producer_claim_next(value);
		// }
		*value = &(data[idx]);
		// volatile uint _next = next_to_be_claimed;
		// printf("claimed idx %lu -- claimed %lu -- head %u -- next %u -- thread_id %u\n", 
		// 	idx, _claimed, _head, _next,std::this_thread::get_id());
		// while(!std::atomic_compare_exchange_strong(&claimed, &_claimed, (_claimed+1)&ring_mask));
		return (int64_t) idx;
		// return MPSC_QUEUE_FULL;
	}
	uint producer_publish(volatile int64_t _idx){
		uint _claimed = claimed.fetch_sub(1) - 1;
		volatile uint _head = head;		
		uint idx = _idx; 
		uint next = next_to_be_claimed;

		// while(!std::atomic_compare_exchange_strong(&claimed, &_claimed, (_claimed-1)&ring_mask));
		if( _claimed == 0){
			if( _head != next){
				head = next;
				//while(!std::atomic_compare_exchange_strong(&head, &next, (next+1)&ring_mask)  );
			}
			// else if(_head > idx){
			// 	while(!std::atomic_compare_exchange_strong(&head, &idx, (idx+1)&ring_mask)  );
			// } 

			// while( !head.compare_exchange_strong( (uint *) &idx, idx + 1,std::memory_order_seq_cst,std::memory_order_seq_cst ) );
	
			uv_async_send(&uv_async_handle);


			// if empty, wake up consumer thread;
			// if( ( _head == (tail+1) || (tail==ring_mask && _head==0) )){
			// }

		}

		_head = head;
		volatile uint _next = next_to_be_claimed;
		// printf("publish idx %u -- claimed %u -- head %u -- next %u -- thread id %u\n", 
		// 		idx, _claimed , _head, _next, std::this_thread::get_id());
		std::this_thread::yield();		        

		return 0;

	}
private:
	T *data;
	std::atomic<uint> claimed;
	std::atomic<uint> next_to_be_claimed;
	std::atomic<uint> head; 
	volatile uint tail;
	uint ring_size;
	uint ring_mask;
	uint mask; 
	mpsc_queue_t(const mpsc_queue_t&) = delete;
	void operator=(const mpsc_queue_t&) = delete;
};
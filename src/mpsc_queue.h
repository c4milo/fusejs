#define MPSC_QUEUE_EOF -1
#define MPSC_QUEUE_FULL -2
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>

/* 
 
This is a very simple MPSC queue using a ring_buffer.
This is meant to be used in conjunction with fusejs

*/

template <typename T>
class mpsc_queue_t
{
public:
	mpsc_queue_t(uint ring_size) {
		data = (T*) malloc(ring_size * sizeof(T));
		if(data == nullptr){
			//TODO: thow error about not enough memory;
			// printf("WE WERE NOT ABLE TO ALLOCATE MEMOMRY FOR THE QUEUE! \n");
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

	int consume(T **value){
		/* 
		This function returns the next item to be consumed
		*/
		mutex_lock.lock();
		// printf("consume lock\n");
		if( isEmpty()){
			// printf("consume unlock\n");
			mutex_lock.unlock();
			return MPSC_QUEUE_EOF;			
		}

		uint idx = (tail + 1 ) & ring_mask;
		tail = idx;

		*value = &(data[idx]);
		// printf("consume unlock\n");
		mutex_lock.unlock();

		return idx;

	}

	bool isEmpty(){
		return head == tail;
	}

	bool isFull(){
		
		/*
			the queue is full when next_to_be_claimed + 1 is tail.
			if we defined full as next_to_be_claimed == tail, it could lead to situation where we enqueue something 
			at the tail position without consuming it.
		*/
		return ( 
			(next_to_be_claimed + 1) & ring_mask 
		) == tail;

	}

	int producer_claim_next(T **value){
		mutex_lock.lock();
		// printf("next lock\n");

		while ( isFull() )
		{

	        // the queue is full		      
	        // wait until the queue clears
			// printf("next unlock\n");
			mutex_lock.unlock();
			uv_async_send(&uv_async_handle);
			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			mutex_lock.lock();				
			// printf("next lock\n");

		} 

		#ifdef DEBUG
		uint _claimed =  ++claimed;
		#else
		claimed++;
		#endif

		uint idx = (next_to_be_claimed++) & ring_mask;

		// printf("next unlock\n");
		mutex_lock.unlock();
		*value = &(data[idx]);

		#ifdef DEBUG
		volatile uint _head = head;
		volatile uint _next = next_to_be_claimed;
		// printf("claimed idx %lu -- claimed %lu -- head %u -- next %u -- tail %u -- thread_id %u\n", 
			idx, _claimed, _head, _next, tail, std::this_thread::get_id());
		#endif

		// while(!std::atomic_compare_exchange_strong(&claimed, &_claimed, (_claimed+1)&ring_mask));
		return (int) idx;
	}
	uint producer_publish(int idx){
		mutex_lock.lock();
		// printf("publish lock\n");
		claimed--;

		// set a new head when the number claimed is 0
		if( claimed == 0){
			
			head = (next_to_be_claimed + ring_mask - 1)  & ring_mask;		
			// if( head != next){
			// 	head = next;
			// 	//while(!std::atomic_compare_exchange_strong(&head, &next, (next+1)&ring_mask)  );
			// }

			// printf("publish unlock\n");
			mutex_lock.unlock();

			uv_async_send(&uv_async_handle);



		}else{
			// printf("publish unlock\n");
			mutex_lock.unlock();
		}

		#ifdef DEBUG
		uint idx = _idx; 
		_head = head;
		volatile uint _next = next_to_be_claimed;
		// printf("publish idx %u -- claimed %u -- head %u -- next %u -- tail %u -- thread id %u\n", 
			idx, _claimed , _head, _next, tail, std::this_thread::get_id());
		#endif

		std::this_thread::yield();		        
		return 0;

	}
private:
	T *data;
	std::mutex mutex_lock;
	// std::atomic<uint> claimed;
	// std::atomic<uint> next_to_be_claimed;
	// std::atomic<uint> head; 
	uint claimed;
	uint next_to_be_claimed;
	uint head; 
	uint tail;
	uint ring_size;
	uint ring_mask;
	uint mask; 
	mpsc_queue_t(const mpsc_queue_t&) = delete;
	void operator=(const mpsc_queue_t&) = delete;
};
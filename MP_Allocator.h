#pragma once
#pragma once
#include<cstddef>
/*结构：
Free_List[1] -> Free_List[2] -> ... ->Free_List[Max_Length]
||
\/
node
...
*/
#define Align 2048				//2048bit =32Bytes 
#define Max_Bytes 1024			//节点最大存储空间
#define Max_Length 20			//自由链表上的长度为20
#define Wrong_Malloc false		//操作系统内存分配失败异常抛出标志

//链表上的节点
struct node
{
	node* next;					//指向下一个节点
	char* value;				//可用的空间
};

static node* Free_List[Max_Length] = { nullptr };		//总链表
static char* Start_Of_Pool;		//内存池的首地址
static char* End_Of_Pool;		//内存池的末地址
static size_t Pool_Size = 16;

template<class T>
class MP_Allocator
{
public:
	typedef void _Not_user_specialized;
	typedef T value_type;							//T
	typedef value_type *pointer;					//T*
	typedef const value_type *const_pointer;	    //const T*
	typedef value_type& reference;				    //T&
	typedef const value_type& const_reference;		//const T&
	typedef size_t size_type;						//unsigned类型，用于指明数组长度或下标，它必须是一个正数
	typedef ptrdiff_t difference_type;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type is_always_equal;
	
	pointer address(reference _Val) const _NOEXCEPT;
	const_pointer address(const_reference _Val) const _NOEXCEPT;
	void deallocate(pointer _Ptr, size_type _Count);		//_Count表示删除的个数
	_DECLSPEC_ALLOCATOR pointer allocate(size_type _Count);	//传输元素T的个数，返回链表上大小合适的指针

	//释放对象但不释放对象所占用的空间
	template<class _Objty>
	void destroy(_Objty *_Ptr)
	{
		_Ptr->~_Objty();
	}
	//根据给定空间和地址构造对象
	template<class _Objty, class... _Types>
	void construct(_Objty* _Ptr, _Types&&... _Args)
	{
		new(_Ptr) _Objty(std::forward<_Types>(_Args)...);
	}

	//构造函数
	MP_Allocator() _NOEXCEPT {}
	template<class _Uty>
	MP_Allocator(const MP_Allocator<_Uty>& refAllocator)_NOEXCEPT:MP_Allocator() {}
	//析构函数
	~MP_Allocator() {}

private:
	//辅助函数
	//给定一个大小(byte)，将它按align(byte)对齐
	static size_type Round_Up(size_t memory_size) 
	{
		return ((memory_size + Align - 1) & ~(Align - 1));
	}

	//根据所需的，或多余的空间大小(Byte)，计算合适的自由链表下标
	static size_type Get_Free_List_Index(size_t memory_size) 
	{
		return ((memory_size + Align - 1) / (Align - 1));
	}

	//主要的内存分配函数
	//根据所需的空间大小(Byte)，在内存池中寻找相应的空间，将多个节点挂在链表上，并返回一个memory_size(bytes)大小空间的指针
	static void* refill(size_t memory_size);
	//给定单个节点的大小size, 以及分配的节点数num，
	//如果内存池不足以提供num个节点， 将num修改为可能的最大值
	static char* blockAlloc(size_type size, int& num);
};

//传入一个对象，返回其地址
template<class T>
T* MP_Allocator<T>::address(reference _Val) const _NOEXCEPT
{
	return (T*)(&_Val);
}

//传入一个对象引用，返回其地址
template<class T>
const T* MP_Allocator<T>::address(const_reference _Val) const _NOEXCEPT
{
	return (T*)(&_Val);
}

//传输元素T的个数，返回链表上大小合适的指针
template<class T>
T* MP_Allocator<T>::allocate(size_type _Count)
{
	//如果所需的空间(Byte)大于链表上最大的空间(Byte),则直接malloc
	if (_Count * sizeof(value_type) > Max_Bytes)
	{
		pointer get_malloc;
		//检验操作系统分配内存是否成功，如果失败抛出异常
		try
		{
			if ((get_malloc = (pointer)(malloc(_Count * sizeof(value_type)))) != nullptr)
				return get_malloc;
			else
				throw(Wrong_Malloc);
		}
		catch (bool)
		{
			std::cout << "ERROR: get 'NULL' from '(pointer)(malloc(_Count * sizeof(value_type)))'" << std::endl;
			std::cout << "FORCED EXIT!";
			exit(0);					//强制结束程序
		}
	}
	//等待返回的指针
	node* ptr = Free_List[Get_Free_List_Index(_Count * sizeof(value_type))];
	node**Ptr_Address = &ptr;			//指针的地址
	
	if (ptr)							//如果指针非空，说明指针的确指向一片内存
	{
		*Ptr_Address = ptr->next;		//自由链表指向下一个节点
		return (pointer)ptr;			//返回指针
	}
	else								//返回为空，说明链表上的节点已经用完，调用refill，向列表补充节点
	{								
		size_type size = _Count * sizeof(value_type);		//计算分配的每个节点的空间大小(byte)
		size = Round_Up(size);			//将size对齐
		return (pointer)refill(size);	//重新填充list
	}
}

//根据所需的空间大小(Byte)，在内存池中寻找相应的空间，将多个节点挂在链表上，并返回一个memory_size(bytes)大小空间的指针
template<class T>
void* MP_Allocator<T>::refill(size_t memory_size)
{
	int i = 0;							//循环变量
	node *curnode, *nextnode;			//循环更替以完成链表的连接
	int num = Max_Length;
	char* block = blockAlloc(memory_size, num);				//如果内存池不足以提供num个节点， 将num修改为可能的最大值
															//在链表上找到挂节点的地址
	node** ptr_to_list = Free_List + Get_Free_List_Index(memory_size);

	if (num == 1) 
	{
		return (void*)block;			//如果内存池只能提供一个节点，那么直接返回
	}
	else 
	{
		*ptr_to_list = (node*)(block + memory_size);
		nextnode = (node*)(block + memory_size);
		//如果内存池提供了多个节点，将他们连成链表
		while (i != num - 2)			//当连完n-2个节点后退出
										//注：-2是因为num个空间，一个直接返回给用户，一个直接挂在list上
		{			
			curnode = nextnode;
			nextnode = (node*)((char*)curnode + memory_size);
			curnode->next = nextnode;
			i++;
		}
		nextnode->next = NULL;
		return (void*)block;
	}
}

//给定单个节点的大小size, 以及分配的节点数num，
//如果内存池不足以提供num个节点， 将num修改为可能的最大值
template<class T>
char* MP_Allocator<T>::blockAlloc(size_type size, int& num)
{
	char* ret;							//待返回变量
	size_type bytesNeeded = size * num;
	size_type bytesLeft = End_Of_Pool - Start_Of_Pool;
	if (bytesLeft >= bytesNeeded)		//如果剩下的空间大于所需的空间，将内存池的首地址返回
	{		
		ret = Start_Of_Pool;
		Start_Of_Pool += bytesNeeded;
		return ret;
	}
	else if (bytesLeft > size)			//内存池不足以提供num个节点， 将num修改为可能的最大值
	{		
		num = bytesLeft / size;
		ret = Start_Of_Pool;
		Start_Of_Pool += num * size;
		return ret;
	}
	else 
	{
		size_type Bytes_To_Get = 5 * bytesNeeded + Round_Up((Pool_Size >> 4));
		if (bytesLeft > 0)				//将剩余的空间放回链表上
		{			
			node** Ptr_To_List = Free_List + Get_Free_List_Index(bytesLeft);
			((node*)Start_Of_Pool)->next = *Ptr_To_List;
			*Ptr_To_List = (node*)Start_Of_Pool;
		}
		try
		{
			if ((Start_Of_Pool = (char*)(malloc(Bytes_To_Get))) == NULL)		//内存池向系统申请内存
				throw(Wrong_Malloc);	//抛出异常
		}
		catch(bool)						//如果malloc返回空指针,在链表上找合适大小的空间
			{	
				std::cout << "WARNING: get 'NULL' from '(pointer)(malloc(_Count * sizeof(value_type)))'" << std::endl;
				node**Ptr_To_LIST, *p;
				for (int i = size; i <= Max_Bytes; i += Align)
				{
					Ptr_To_LIST = Free_List + Get_Free_List_Index(i);
					p = *Ptr_To_LIST;
					if (p)					//如果p，不为空，链表上有对应的节点
					{
						Start_Of_Pool = (char*)p;				//将节点回收到内存池
						End_Of_Pool = (char*)(Start_Of_Pool + i);
						*Ptr_To_LIST = p->next;
						return(blockAlloc(size, num));			//重新分配
					}
				}
			}
		Pool_Size += Bytes_To_Get;
		End_Of_Pool = Start_Of_Pool + Bytes_To_Get;
		return blockAlloc(size, num);
	}
}

//释放内存函数
template<class T>
void MP_Allocator<T>::deallocate(pointer _Ptr, size_type _Count)
{
	if (_Count == 0) return;
	if ((_Count * sizeof(value_type)) > Max_Bytes)	free(_Ptr);//如果删除空间过大，直接delete;
	else									//放回链表中
	{								
		node**Ptr_To_List = Free_List + Get_Free_List_Index(_Count * sizeof(value_type));
		((node*)_Ptr)->next = *Ptr_To_List;
		*Ptr_To_List = (node*)_Ptr;
	}
}
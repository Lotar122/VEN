namespace nihil::nstd
{
	template<typename T>
	class SingleLink
	{
		T data;
		SingleLink* pNext;
	};

	template<typename T>
	class DoubleLink : SingleLink<T>
	{
		DoubleLink* pPrev;
	};

	template<typename T>
	class Node
	{
		T node;
	};

	//!!!UNFINISHED!!!
	//U is the data type, while T is the Link type, put the U as the template argument for the T
	template<typename U, typename T = SingleLink<U>>
	class LinkedList
	{
	private:
		Node<T>* head = NULL;
	public:
		void append(U data)
		{
			Node* newNode = new Node<T>();
			newNode->data = data;
			newNode->pNext = NULL;

			if (head == NULL)
			{
				head = newNode;
			}
			else
			{
				Node* temp = head;
				while (temp->pNext != NULL)
				{
					temp = temp->pNext;
				}
				temp->next = newNode;
			}
		}

		void prepend(U data)
		{
			Node* newNode = new Node<T>();
			newNode->data = data;
			newNode->next = head;
			head = newNode;
		}
	};
}
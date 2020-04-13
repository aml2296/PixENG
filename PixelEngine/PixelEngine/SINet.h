#pragma once
#include <SDL.h>
class CollisionBox
{
public:
	SDL_Rect pBoundingBox;
	CollisionBox()
	{
		pBoundingBox = SDL_Rect{ 0,0,0,0 };
	};
	CollisionBox(SDL_Rect r)
	{
		pBoundingBox = r;
	}
	static bool CollisionCheck(SDL_Rect a, SDL_Rect b);
	virtual void OnCollision(void *ptr)
	{
		
	};
	virtual void OnColliding(CollisionBox &other) {};
	virtual void OnExitCollision(CollisionBox &other) {};
};
bool CollisionBox::CollisionCheck(SDL_Rect a, SDL_Rect b)
{
	if ((a.x <= b.x + b.w && a.x >= b.x) || (a.w + a.x <= b.w + b.x && a.w + a.x >= b.x))
		if ((a.y <= b.y + b.h && a.y >= b.y) || (a.h + a.y <= b.h + b.y && a.h + a.y >= b.y))
			return true;
	return false;
}

enum class SILayerIgnore
{

};
class SINode
{
private:
	const static int capp = 5;
public:
	bool open = true;
	float charge = 0.0f;
	CollisionBox* cBox = nullptr;
	float* radius = nullptr;
	SINode* connections = nullptr;
	int count = 0;
	SINode& operator=(SINode& other)
	{
		open = other.open;
		charge = other.charge;
		cBox = other.cBox;
		radius = other.radius;
		connections = other.connections;
		count = other.count;
		return *this;
	}
	void Reset()
	{
		open = false;
		charge = 0.0f;
		cBox = nullptr;
		radius = nullptr;
		connections = nullptr;
		count = 0;
	}
	static int Capacity() { return capp; };
};
class SINet
{
private:
	SINet* nextNet = nullptr;
	SINode* network = nullptr;
	SINode* xMin = nullptr;
	SINode* yMin = nullptr;
	SINode* xMax = nullptr;
	SINode* yMax = nullptr;
	float radius = 5.0f;
	static const int points = 16;
	static const double pi() { return 3.145927; };

	unsigned int Cap = 100;
	int count = 0;
	void MinMaxNode(SINode* n)
	{
		if (xMinCheck(*n))
			xMin = n;
		if (yMinCheck(*n))
			yMin = n;
		if (yMaxCheck(*n))
			yMax = n;
		if (xMaxCheck(*n))
			xMax = n;
	}
	int SystemItemCount()
	{
		int i = 0;
		SINet* temp = this;
		while (temp != nullptr)
		{
			i += temp->count;
			temp = temp->nextNet;
		}
		return i;
	}
	int SystemCount(int& t)
	{
		t += count;
		if (nextNet)
			return nextNet->SystemCount(t) + 1;
		else
			return 1;
	}
	bool xMinCheck(SINode n)
	{
		if (xMin == nullptr || n.cBox->pBoundingBox.x < xMin->cBox->pBoundingBox.x)
			return true;
		return false;
	}
	bool yMinCheck(SINode n)
	{
		if (yMin == nullptr || n.cBox->pBoundingBox.y < xMin->cBox->pBoundingBox.y)
			return true;
		return false;
	}
	bool xMaxCheck(SINode n)
	{
		if (xMax == nullptr || n.cBox->pBoundingBox.x + n.cBox->pBoundingBox.w > xMax->cBox->pBoundingBox.x + xMax->cBox->pBoundingBox.w)
			return true;
		return false;
	}
	bool yMaxCheck(SINode n)
	{
		if (yMax == nullptr || n.cBox->pBoundingBox.y + n.cBox->pBoundingBox.h > yMax->cBox->pBoundingBox.y + yMax->cBox->pBoundingBox.h)
			return true;
		return false;
	}
	SINode* CheckForBreak(SINode* n) //returns Nullptr on Fail and the Address of Split Node on Success
	{
		for (int i = 0; i < n->count; i++)
			if (RadiusCheck(*n, n->connections[i]) == false)
				return n->connections + i;
		return nullptr;
	}
	bool ConfirmBreak(SINode* A, SINode* B)
	{
		A->charge = 1.0f;
		B->charge = -1.0f;

		if (searchCharge(A, B->charge))
			return false;
		return true;
	}
	bool searchCharge(SINode* n, float charge)
	{
		if (n->charge == charge)
			return true;
		else
		{
			for (int i = 0; i < n->count; i++)
			{
				if (searchCharge(&n->connections[i], charge))
					return true;
				n->connections[i].charge = n->charge;
			}
			return false;
		}
	}
	void setCharge(float f)
	{
		for (int i = 0; network->count; i++)
			network[i].charge = f;
	}
	SINet* Split(SINode* n)
	{
		SINet* newNet = new SINet;
		SplitBranch(n, newNet);
		Clean();
		return newNet;
	}
	bool SplitBranch(SINode* n, SINet* newNet)
	{
		newNet->Insert(n);
		for (int i = 0; i < n->count; i++)
			SplitBranch(n->connections + i, newNet);
		Remove(n);
		return true;
	}
	void Remove(SINode* n)
	{
		n->Reset();
		count--;
	}
	void Insert(SINode* n)
	{
		if (count + 1 > Cap)
		{
			printf("Hit SINode Cap!!!\n");
			//Add room
		}
		else
		{
			n->open = false;
			n->radius = &radius;
			int i;
			for (i = 0; i < count; i++);
			network[i] = *n;
			MinMaxNode(network + i);
			count++;
		}
	}
	void Clean()
	{
		int fill = 0;
		for (int i = 0; i < count; i++)
		{
			while ((network + i + fill)->open && i + fill < Cap)
				fill++;
			network[i - fill] = network[i + fill];
			network[i + fill].Reset();
		}
	}
	void AbsorbNet(SINet* net)
	{
		SINet* netTemp = this;
		for (int i = 0; i < net->count; i++)
		{
			Insert(net->network + i);
			net->Remove(net->network + i);
		}

		while (netTemp->nextNet != net)
		{
			netTemp = netTemp->nextNet;
		}
		netTemp->nextNet = net->nextNet;
		delete net;
	}
	static bool AboveLine(SDL_Point p, SDL_Point a, SDL_Point b) //Where a is the left most point, b is the right most point and p is the point in question
	{
		if (b.y - a.y == 0)
			return false;
		else
		{
			float slopeNum = b.y - a.y;
			float slopeDen = b.x - a.x;
			float slope = slopeNum / slopeDen;

			return p.y - (slope * p.x) > 0;
		}
	}
	static bool RadiusCheck(SINode a, SINode b)
	{
		SDL_Point* aPoints = new SDL_Point[points];
		SDL_Point* bPoints = new SDL_Point[points];
		bool collision = false;
		if (CollisionBox::CollisionCheck(a.cBox->pBoundingBox, b.cBox->pBoundingBox))
		{
			bool TopHalf;
			bool BottHalf;
			SDL_Rect* rect;
			float cosP;
			float sinP;

			for (int i = 0; i < points; i++)
			{
				rect = &a.cBox->pBoundingBox;
				cosP = cos((float)(((360 / points) * i) * (pi() / 180)));
				sinP = sin((float)(((360 / points) * i) * (pi() / 180)));
				aPoints[i] = SDL_Point{
					(int)(*a.radius * cosP) + rect->x + (int)(0.5 * rect->w),
					(int)(*a.radius * sinP) + rect->y + (int)(0.5 * rect->h) };
				rect = &b.cBox->pBoundingBox;
				bPoints[i] = SDL_Point{
					(int)(*b.radius * cosP) + rect->x + (int)(0.5 * rect->w),
					(int)(*b.radius * sinP) + rect->y + (int)(0.5 * rect->h) };
			}

			for (int k = 0; k < points; k++)
			{
				TopHalf = false;
				BottHalf = false;
				for (int i = 0; i < points / 2; i++)
				{
					if (!AboveLine(bPoints[k], aPoints[i], aPoints[i + 1]))
						TopHalf = true;
					if (AboveLine(bPoints[k], aPoints[i + (points / 2)], aPoints[((i * points / 2) + 1) > points ? 0 : i + 1]))
						BottHalf = true;
				}
				if (TopHalf == true && BottHalf == true)
				{
					collision = true;
					break;
				}
			}
			delete[] aPoints;
			delete[] bPoints;
		}
		return collision;
	}
public:
	SINet()
	{
		network = new SINode[Cap];
	};
	~SINet()
	{
		delete[] network;
	}
	int Size() { return count; };
	int TotalSize()
	{
		int t = 0;
		return t + SystemCount(t);
	};
	SDL_Rect GetRect()
	{
		SDL_Rect returnRect = { 0,0,0,0 };
		if (xMax && yMax && xMin && yMin)
		{
			returnRect.x = xMin->cBox->pBoundingBox.x - radius;
			returnRect.y = yMin->cBox->pBoundingBox.y - radius;
			returnRect.w = abs((xMax->cBox->pBoundingBox.x + xMax->cBox->pBoundingBox.w + 2 * radius) - returnRect.x);
			returnRect.h = abs((yMax->cBox->pBoundingBox.y + yMax->cBox->pBoundingBox.h + 2 * radius) - returnRect.y);
		}
		return returnRect;
	}
	void setRadius(int i)
	{
		if (i > 0)
			radius = i;
		else
			radius = 5;
	}
	void Insert(CollisionBox *c)
	{
		SINode n;
		n.cBox = c;
		if (count < 1 || CollisionBox::CollisionCheck(c->pBoundingBox, GetRect()))
			Insert(&n);
		else
		{
			if (nextNet)
				nextNet->Insert(c);
			else
			{
				nextNet = new SINet();
				nextNet->radius = radius;
				nextNet->Insert(c);
			}
		}
	}
	void RunCollisionCheck()
	{
		//Check for a Split in the Network
		SINet* SplitNet = nullptr;
		for (int i = 0; i < count; i++)
		{
			SINode* breakNode = CheckForBreak(network + i);
			if (breakNode && ConfirmBreak(network + i, breakNode)) //if a SINode loses a connection and is no longer connected to the Network
			{															// Then a new network is created from the old
				i = 0;
				if (SplitNet != nullptr)
				{
					SINet* tempNet = SplitNet;
					while (tempNet->nextNet != nullptr)
						tempNet = tempNet->nextNet;
					tempNet->nextNet = Split(network + i);
				}
				else
					SplitNet = Split(network + i);
			}
		}
		//Splits have been handled
		//Check for collisions in Network and check for new Master Nodes
		for (int i = count - 1; i >= 0; i--)
		{
			MinMaxNode(network + i);
			for (int k = i - 1; k >= 0; k--)
			{
				if (CollisionBox::CollisionCheck(network[i].cBox->pBoundingBox, network[k].cBox->pBoundingBox))
				{
					network[i].cBox->OnCollision(network[k].cBox);
					network[k].cBox->OnCollision( network[i].cBox);
				}
			}
		}
		//Check for Colliding Networks and Combine if they do
		if (nextNet)
		{
			SINet* temp = nextNet;
			while (temp != nullptr)
			{
				if (CollisionBox::CollisionCheck(GetRect(), temp->GetRect()))
				{
					AbsorbNet(temp);
					temp = this;
				}
				temp = temp->nextNet;
			}
			//Add New Networks from Split
			//temp->nextNet = SplitNet;
		}
		else if (SplitNet)
			nextNet = SplitNet;

		//Check other Networks
		if (nextNet)
			nextNet->RunCollisionCheck();
	}
	SDL_Rect** rectsInSystem()
	{
		SDL_Rect** rectArray = new SDL_Rect * [count];
		for (int i = 0; i < count; i++)
			rectArray[i] = &network[i].cBox->pBoundingBox;
		return rectArray;
	}
	SDL_Rect** rectsInTotalSystem()
	{
		int total = 0;
		total += SystemCount(total);
		SINet* temp = this;
		SDL_Rect** rectArray = new SDL_Rect * [total];

		int p = 0;
		while (temp != nullptr)
		{
			for (int i = 0; i < temp->count; i++, p++)
				rectArray[p] = &temp->network[i].cBox->pBoundingBox;
			rectArray[p++] = new SDL_Rect(temp->GetRect());
			temp = temp->nextNet;
		};
		return rectArray;
	}
	SDL_Point* RadiusInSystem(int& length)
	{
		length = count * points;
		SDL_Point* pArray = new SDL_Point[length];
		SDL_Rect* rect = nullptr;
		float cosP = 0;
		float sinP = 0;
		for (int i = 0; i < count; i++)
			for (int p = 0; p < points; p++)
			{
				rect = &network[i].cBox->pBoundingBox;
				cosP = cos((float)(((360 / points) * p) * (pi() / 180)));
				sinP = sin((float)(((360 / points) * p) * (pi() / 180)));
				pArray[(i * p) + p] = SDL_Point{
					(int)(*network->radius * cosP) + rect->x + (int)(0.5 * rect->w),
					(int)(*network->radius * sinP) + rect->y + (int)(0.5 * rect->h)
				};
			}
		return pArray;
	}
	SDL_Point* RadiusInTotalSystem(int& length)
	{
		length = 0;
		SINet* temp = this;
		SDL_Rect* rect = nullptr;
		int t = SystemItemCount() * points;
		SDL_Point* pArray = new SDL_Point[t];
		int i = 0;
		float cosP = 0;
		float sinP = 0;
		int x = 0;
		int y = 0;

		while (temp != nullptr)
		{

			for (; i < length + temp->count; i++)
			{
				rect = &temp->network[i - length].cBox->pBoundingBox;
				for (int p = 0; p < points; p++)
				{
					cosP = cos((float)(((360 / points) * p) * (pi() / 180)));
					sinP = sin((float)(((360 / points) * p) * (pi() / 180)));
					x = (int)(temp->radius * cosP) + rect->x + (int)(0.5 * rect->w);
					y = (int)(temp->radius * sinP) + rect->y + (int)(0.5 * rect->h);
					pArray[(i * points) + p] = SDL_Point{ x,y };
				}

			}
			length += temp->count;
			temp = temp->nextNet;
		}
		length *= points;
		return pArray;
	}
};
class SIMasterNode
{
public:
	SINet* data = nullptr;
	SIMasterNode* next = nullptr;
	SDL_Color pen = SDL_Color{ 255,0,0 };
	int layer = 0;

	SIMasterNode(int l = 0, SINet* d = nullptr, SIMasterNode* n = nullptr, int* ignore = nullptr)
	{
		data = d;
		next = n;
		layer = l;
	};
};
class SIMaster
{
private :
	SIMasterNode* head;
	SIMasterNode* tail;

	SIMasterNode* FindCollisionBox(SDL_Rect* rec);
public:
	void InsertCollisionBox(CollisionBox* n, int layer);
	void RemoveCollisionBox(CollisionBox* n);
	void CollisionCheck();
	void SetColor(SDL_Color c, int layer);
};

SIMasterNode* SIMaster::FindCollisionBox(SDL_Rect* rec)
{
	SIMasterNode* temp = head;
	SDL_Rect** recSystem;
	while (temp != nullptr)
	{
		recSystem = temp->data->rectsInSystem();
		for (int i = 0; i < temp->data->Size(); i++)
			if (recSystem[i] == rec)
				return temp;
		delete recSystem;
		temp = temp->next;
	}
	return nullptr;
}
void SIMaster::InsertCollisionBox(CollisionBox* n, int layer)
{
	if (!head)
	{
		head = new SIMasterNode(layer,new SINet());
		head->data->Insert(n);
		tail = head;
	}
	else
	{
		if (head->layer == layer)
			head->data->Insert(n);
		else
		{
			if (head->layer > layer)
			{
				SIMasterNode* temp = new SIMasterNode(layer, new SINet());
				temp->data->Insert(n);
				temp->next = head;
				head = temp;
			}
			else
			{
				if (tail->layer == layer)
					tail->data->Insert(n);
				else
				{
					if (tail->layer < layer)
					{
						tail->next = new SIMasterNode(layer, new SINet());
						tail = tail->next;
						tail->data->Insert(n);
					}
					else
					{
						SIMasterNode* temp = head;
						while (temp->next && layer > temp->next->layer)
							temp = temp->next;
						if (temp->layer == layer)
							temp->data->Insert(n);
						else
						{
							SIMasterNode* newNode = new SIMasterNode(layer, new SINet());
							newNode->data->Insert(n);
							newNode->next = temp->next;
							temp->next = newNode;
						}
					}
				}
			}
		}
	}
}
void SIMaster::RemoveCollisionBox(CollisionBox *n)
{
	SIMasterNode* temp = head;
	SIMasterNode* pos = FindCollisionBox(&n->pBoundingBox);
	if (!pos)
		return;

	while (temp->next != pos)
		temp = temp->next;
	temp->next = pos->next;
	delete pos;
}
void SIMaster::CollisionCheck()
{
	SIMasterNode* temp = head;
	while (temp)
	{
		temp->data->RunCollisionCheck();
		temp = temp->next;
	}
}

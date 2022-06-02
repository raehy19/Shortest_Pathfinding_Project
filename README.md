# Shortest_Pathfinding_Project

Shortest Pathfinding program using A* algorithm

## 1. Abstract
신촌 가게들의 위치 정보의 저장과 활용을 주제로 프로젝트를 진행하였다. 지도데이터는 BMP 이미지 파일로 불러오고, 가게의 유형과 지도상 위치는 txt 파일에서 불러온다. 에이스타 알고리즘을 이용하여 입력한 목적지 또는 가장 가까운 유형의 가게를 찾아, 갈 수 있는 최단 거리 경로를 구하고, 구한 경로 결과를 지도 이미지 파일에 입력하여 저장한다. 



## 2. Data
### 2.1.1 Collecting Data – Map.bmp
![image](https://user-images.githubusercontent.com/69339099/171673866-1c1ac80c-8b67-4897-a4e8-a8d32b8c31e8.png)

지도 데이터이다. 길과 갈 수 없는 곳을 bmp이미지를 이용하여 이용한다. 인터넷에서 수집한 지도 사진을 변형하였다. 모든 픽셀은 rgb 0 0 0과 rgb 255 255 255인 검은색과 흰색 픽셀들로 이루어져 있다. 이 프로젝트 프로그램은 픽셀이 흰색이면 길이라고 판단한다. 가로 500 세로 235픽셀의 지도이므로, 이 지도로 대략 12만개 노드 크기의 메모리를 할당하여 연산을 진행하게 된다. 이미지의 크기가 클 경우, 그만큼 많은 메모리를 사용하게 되고, 길과 경로 정보를 표현하기엔 이정도의 해상도도 충분하다고 생각되어 이미지의 크기를 줄여서 사용했다.



### 2.1.2 Collecting Data – point_list.txt
순서대로 띄어쓰기로 구분된, 가게의 종류와 이름, 이 프로그램 지도상의 위치정보를 담고 있는 데이터이다.
```
장소 빨간잠망경 194 97
장소 신촌역1번출구 182 8
장소 신촌역2번출구 197 13
장소 신촌역3번출구 208 13
장소 신촌역4번출구 224 6
카페 독수리다방 212 217
카페 샐리스도넛 172 154
음식점 육회본가 132 170
음식점 대포찜닭 200 170
음식점 김판석초밥 92 204
음식점 화로상회 337 218
음식점 기꾸스시 112 97
음식점 마녀주방 252 165
술집 아무 139 223
술집 연대포 127 147
술집 타이완쏘야 126 143
술집 달팽이 92 178
```


### 2.2 Choosing Data Structure
가게의 종류와 이름, 위치정보를 데이터를 불러올 때 1차원 배열 class 사용
-> for문 하나로 간단하게 모든 정보를 표처럼 표현하기에 용이

지도의 데이터를 x-y 2축 좌표계로 불러올 때 2차원 배열 struct 사용
-> 2차원 평면의 점들을 2차원 배열로 나타내기에 적절

A* 알고리즘을 진행하기 위해 지도를 노드들로 변환할 때 2차원 배열 class 사용
-> 2차원 평면의 노드들로 거리를 구하기에 용이

A* 알고리즘 열린 리스트에 노드 포인터를 저장할 때, 1차원 포인터 배열을 이용한 최소 힙 heap 사용
-> 노드를 판단하여 열린 리스트에 넣고 f값이 가장 작은 노드를 꺼내는 것을 반복하는 알고리즘에서, 최소 힙을 이용하면 넣을 때 이미 정렬이 완료되기에 연산량을 줄일 수 있음

A* 알고리즘 닫힌 리스트에 노드 포인터를 저장할 때, 1차원 포인터 배열 사용
-> 닫힌 리스트는 열린 리스트와 달리 우선순위가 없어서, 노드 검색 시 닫힌 리스트에 있는지 판단하려면 어차피 모든 노드를 검색해야 하므로 가장 간단한 배열 구조 사용



## 3. Functions
### 3.1  point_list.txt 에서 정보를 불러오기 위한 struct : Point
```cpp
// Point struct
typedef struct {
	string name;
	string type;
	int x;
	int y;
} Point;

// 입력받은 이름의 파일에서 pointlist를 불러오는 함수
void load_Point_list(const char* Fname, Point* list, int* list_idx)

// pointlist를 출력하는 함수
void Print_Point_list(Point* list, int list_idx)
```
띄어쓰기로 구분된, 가게의 종류와 이름, 이 프로그램 지도상의 위치정보를 담고 있는 데이터를 불러와 저장할 struct : Point.
load_Point_list는 pointlist를 불러오는 함수, Print_Point_list는 화면에 출력하는 함수이다.



### 3.2  BMP지도를 읽어와 헤더와 픽셀 정보를 저장할 Class : BMPImage
```cpp
// BMPImage class
class BMPImage {
public:
	BMPHeader header;
	Pixel** pixels;
	int height;
	int width;

	BMPImage() {
		this->height = 0;
		this->width = 0;
		this->pixels = NULL;
		set_header("dummy.bmp");
	}

	~BMPImage() {}

	// 빈 픽셀을 만드는 함수
	void create_empty_pixels(int height, int width)

	// 입력받은 이름의 파일을 불러오는 함수
	int load_bmp_image(const char* fName)

	// 파일을 저장하는 함수
	int save_bmp_image(const char* fName)
  
	// dummy.bmp를 이용한 헤더 생성 함수
	int set_header(const char* fName)
};

```
이미지를 로드하고 저장하는데 사용되는 클래스와 함수이다. bmp형식의 지도데이터를 불러오고, 경로가 입력된 이미지를 다시 저장하는 일을 한다.



### 3.3  A* 알고리즘 구현을 위한 Class : Node
```cpp
// class Node
class Node {
private:
	Node* mother;
public:
	int x, y;
	int f, g, h;

	// 생성자
	Node() {
		x = 0;
		y = 0;
		f = 0;
		g = 0;
		h = 0;
		this->mother = NULL;
	}

	~Node() {}

	//노드의 부모노드를 return 하는 함수
	Node* getmother()

	//노드의 부모노드를 재설정 하는 함수
	void setmother(Node* new_mother)
};

//  노드 메모리 할당 함수
Node** create_node_2D(int height, int width)
```
노드의 위치정보인 x, y와, 시작점부터 선택된 노드까지 이동하는데 필요한 비용 g, 이 노드부터 목적지까지의 예상 이동 비용 h, 시작점부터 목적지까지의 예상 이동 비용인 f(=g + h)값과 최소비용으로 이 노드로 이동하였을 때 이 노드의 이전 노드를 가리키는 노드 포인터 mother로 구성되어 있다.



### 3.4  A* 알고리즘 중 우선순위 큐에 사용되는 heap* array 관련 fucntion
```cpp
// 최소 heap에 새 노드를 삽입하는 함수
void heap_push(Node** heap, Node* data, int* heap_idx)

// heap의 루트 노드(heap[1])를 추출하고 남은 노드들을 정렬하는 함수
Node* heap_pop(Node** heap, int* heap_idx)
```
힙의 마지막 노드에 데이터를 삽입하고 정렬하는 함수 heap_push, 힙의 루트노드를 꺼내고 마지막 노드를 루트노드에 넣어서 다시 정렬하는 함수 heap_pop이다.



### 3.5  노드를 판단하여 heap에 추가하거나 정보를 수정하는 함수 : get child
```cpp
// idx_node에 인접한 노드를 판단하여 heap에 추가하거나 정보를 수정하는 함수
void get_child(int child_x, int child_y, int move_cost, Node* idx_node, Node** Mapnode, BMPImage map,  Node** heap, int* heap_idx, Node** closed, int* closed_idx , int dest_x, int dest_y)
```
함수의 입력 child_x와 child_y 는 판단하는 이 노드의 좌표값이고, move_cost는 이 노드가 기존 노드에 수직으로 연결되어 있는지 대각선으로 연결되어 있는지에 따라 수직일경우 10, 대각선일 경우 14를 입력받는다. idx_node는 이 노드에 인접한 이전 노드이다. Mapnode는 노드들의 2차원 배열이고, map은 지도데이터, heap과 closed는 각각 열린리스트와 닫힌 리스트이고 heap_idx와 closed_idx는 리스트의 크기를 나타내는 변수이다. dest_x와 dest_y는 각각 목적지의 x, y좌표이다.
추가하려는 노드가 맵 안의 좌표인지 판단하고, 닫힌 리스트를 검색하여 닫힌 리스트에 없는 길인지 판단하고, Pixel 값을 읽어 갈 수 있는 길인지 판단한다. 그 후 열린 리스트에 있는 노드일 경우 g값을 비교하여 기존 g값보다 idx_node를 통해서 갈 때의 g값이 더 짧을 시 노드 정보를 업데이트 한다.
열린 리스트에 없는 노드일경우 노드클래스의 멤버변수들을 계산하여 입력하고 열린 리스트에 노드를 추가한다.



### 3.6.  A* 알고리즘으로 목적지까지의 최단 거리를 찾는 함수 : find_path
```cpp
// Mapnode를 이용하여 목적지까지 길을 찾는 함수
int find_path(Node** Mapnode, BMPImage* map, int start_x, int start_y, int dest_x, int dest_y)
```
 Mapnode는 A* 알고리즘을 사용하기 위해 만든 class Node의 이중 포인터 배열이다. map은 지도 정보를 가지고 있고, start_x, start_y는 시작점의 좌표, dest_x dest_y는 목적지의 좌표이다.
 열린 리스트 heap과 닫힌 리스트 closed를 선언해준다. 이때 heap은 array로 구현한 최소 힙으로, 첫번째 노드가 heap[0]이 아닌 heap[1]부터 저장된다. 시작점을 생성하고 닫힌 리스트에 넣은 뒤, 시작점 주위 8개의 노드부터 get_child 함수를 이용해 판단하고, 8개의 판단이 끝나면 heap에서 가장 작은 f를 갖는 노드를 꺼내 닫힌 리스트에 넣고 반복하며 최단경로를 찾는다. 최단경로를 찾으면, 그 경로를 map에 그린다.



### 3.7. main()
```
BMPImage Class 선언
  
지도 데이터 불러오기
  
이차원 클래스 배열 선언 및 초기화
  
위치정보를 담을 point list에 메모리 할당
  
point list에 데이터 불러옴
  
Point list에서 찾기 위해 사용할 변수 선언
  
현재 위치와 목적지 입력받음
  
	선택1. 목적지를 입력하면 목적지까지의 경로를 찾고 지도에 나타낸다.

	선택2. 찾으려는 가게의 유형 중 직선거리가 가장 가까운 가게로 가는 경로를 찾고 지도에 나타낸다.
  
```
 사용할 class 배열 등에 필요한 메모리를 할당하고, 위치와 함수 선택에 따라 각각 입력받은 목적지로의 경로를 구해 이미지로 나타내어 저장하거나, 입력받은 가게 유형 중 현재 위치에서 가장 직선거리가 가까운 가게를 찾아 가는 경로를 구해 이미지로 나타내어 저장하는 함수를 실행한다.



## 4. Conclusion
#### 입력 : “신촌역2번출구” “1” “화로상회” 의 프로그램 결과 및 생성된 이미지 파일
![image](https://user-images.githubusercontent.com/69339099/171676200-17726478-404c-45ee-b1be-7fcae17878d9.png)
![image](https://user-images.githubusercontent.com/69339099/171676213-7de54f21-1476-4fe8-a24c-61339cc33327.png)
---
#### 입력 : “달팽이” “1” “마녀주방” 의 프로그램 결과 및 생성된 이미지 파일
![image](https://user-images.githubusercontent.com/69339099/171676242-98c49017-4ac3-49e7-831c-6da29973df58.png)
![image](https://user-images.githubusercontent.com/69339099/171676253-6d7fbd33-ed7d-4ae4-8473-a8effe8e27af.png)
---

#### 장점
지도 데이터를 이용한 연산 길찾기 경로 결과를 다시 이미지 파일로 저장하여 볼 수 있어 직관적이라는 장점이 있다.

#### 단점/개선점 
포인트 리스트를 작성할 때 이 프로그램에 사용되는 지도에 맞게 좌표를 직접 입력해주어야 한다는 단점이 있다. 같은 이유로, 현재 위치를 입력할 때 지도에 맞는 좌표로 변환 위도와 경도를 입력해도 지금 지도의 위치 데이터로 알맞은 스케일로 변환을 해 주는 함수를 추가하면 좋을 것 같다. 또한 point list가 txt 파일이기 때문에 직접 프로그램을 통해서 새로운 point를 추가하거나, 삭제하고, point를 type에 따라 정렬하는 기능도 추가하여 개선하면 좋을 것 같다. 이미지의 색상 값을 이용하면, 한 이미지 지도로 인도, 차도, 자전거도로 만 이용하는 등 교통수단에 따라 갈 수 있는 길을 다르게 설정할 수 있을 것이므로, 지금은 차도와 인도 구분 없이 길을 찾았지만, 교통수단에 따라 다른 길을 찾아낼 수 있도록 개선할 수 있을 것이다.



## 5. References
https://twpower.github.io/137-heap-implementation-in-cpp

https://deliorange.tistory.com/110

#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>
#include <fstream>
using namespace std;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;


///// ///// ///// BMPHeader ///// ///// /////

// BMPFileHeader struct
typedef struct {
	uint8 dummy[2];
	uint8 bfType[2];	//BMP파일 식별을 위한 Magic Number : 0x42, 0x4D
	uint32 bfSize;	//BMP파일 크기
	uint16 bfReserved[2];	//준비. 그림을 그린 응용 프로그램에 따라 다름.
	uint32 bfOffBits;	//Offset. 비트맵 데이터를 찾을 수 있는 시작	
} BMPFileHeader;
// BMPInfoHeader struct
typedef struct {
	uint32 biSize; //현 구조체의 크기
	int32 biWidth; //가로
	int32 biHeight; //세로
	uint16 biPlanes; //사용하는 color plane. 기본 값 : 1
	uint16 biBitCount; //한 화소에 들어가는 비트수. 8bit * RGB = 24
	uint32 biCompression;  //압축 방식
	uint32 biSizeImage; //압축되지 않은 비트맵 데이터의 크기
	int32 biXPelsPerMeter; //그림의 해상도
	int32 biYPelsPerMeter; // 그림의 해상도
	uint32 biClrUsed; //색 팔레트의 색 수, 또는 0에서 기본 값 2^n
	uint32 biClrImportant; //중요한 색의 수. 색이 모두 중요할 경우 0. 일반적으로 무시.
} BMPInfoHeader;
// BMPHeader struct
typedef struct {
	BMPFileHeader fileHeader;
	BMPInfoHeader infoHeader;
} BMPHeader;


///// ///// ///// Pixel ///// ///// /////
// Pixel struct
typedef struct {
	uint8 b;
	uint8 g;
	uint8 r;
} Pixel;
// 픽셀 메모리 할당 함수
Pixel** create_pixels_2D(int height, int width)
{
	Pixel** arr = new Pixel * [height];
	for (int i = 0; i < height; i++)
	{
		arr[i] = new Pixel[width];
	}
	return arr;
}


///// ///// ///// BMPImage ///// ///// /////

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
	void create_empty_pixels(int height, int width){
		this->height = height;
		this->width = width;
		this->pixels = create_pixels_2D(height, width);
		set_header("dummy.bmp");
		cout << "create empty_pixel" << endl;
	}

	// 입력받은 이름의 파일을 불러오는 함수
	int load_bmp_image(const char* fName) {
		//만약 pixels에 정보가 담겨있다면 memory release
		if (this->pixels != NULL) {
			for (int h = 0; h < this->height; h++) {
				delete[] this->pixels[h];
			}
			delete[] this->pixels;
			this->pixels = NULL;
		}

		FILE* file = fopen(fName, "rb");
		fread(&(this->header.fileHeader.bfType[0]), sizeof(uint8), 2, file);
		fread(&(this->header.fileHeader.bfSize), sizeof(uint32), 1, file);
		fread(&(this->header.fileHeader.bfReserved[0]), sizeof(uint16), 2, file);
		fread(&(this->header.fileHeader.bfOffBits), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biSize), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biWidth), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biHeight), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biPlanes), sizeof(uint16), 1, file);
		fread(&(this->header.infoHeader.biBitCount), sizeof(uint16), 1, file);
		fread(&(this->header.infoHeader.biCompression), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biSizeImage), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biXPelsPerMeter), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biYPelsPerMeter), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biClrUsed), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biClrImportant), sizeof(uint32), 1, file);

		int width = this->header.infoHeader.biWidth;
		int height = this->header.infoHeader.biHeight;

		// 비트맵은 가로를 4의 배수로 저장하므로 남는 공간을 채울 padding을 구함
		int numPadding = (4 - ((width * 3) % 4)) % 4;
		uint8 dummy[4];

		// 픽셀 데이터의 시작 위치로 파일포인터 이동
		fseek(file, this->header.fileHeader.bfOffBits, SEEK_SET);

		// pixels에 픽셀 데이터 저장
		this->pixels = create_pixels_2D(height, width);
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				fread(&this->pixels[h][w], sizeof(uint8), 3, file);
			}
			fread(dummy, sizeof(uint8), numPadding, file);
		}
		this->width = width;
		this->height = height;
		fclose(file);
		cout << "BMP File Successfully Loaded. < " << fName << " >" << endl;
		return 0;
	}

	// 파일을 저장하는 함수
	int save_bmp_image(const char* fName) {
		if (this->pixels == NULL) {
			cout << "No Pixel Data. Cannot save bmp image <" << fName << ">." << endl;
			this->width = 0;
			this->height = 0;

			return 1;
		}
		else {
			if (this->height == 0 || this->width == 0) {
				cout << "No Width/Height Data. Cannot save bmp image <" << fName << ">." << endl;
				cout << "Class Crushed. Set pixels, width, height to default values NULL, 0, 0" << endl;
				cout << "Cannot conduct memory releasing. Memory leaking might happened." << endl;
				delete[] this->pixels;
				this->pixels = NULL;
				this->width = 0;
				this->height = 0;

				return 2;
			}
			else {
				int numPadding = (4 - ((this->width * 3) % 4)) % 4;
				uint8 dummy[4] = { 0 };
				this->header.infoHeader.biSizeImage = (this->width * 3 + numPadding) * this->height;
				this->header.fileHeader.bfSize = this->header.infoHeader.biSizeImage + 54;
				this->header.infoHeader.biWidth = this->width;
				this->header.infoHeader.biHeight = this->height;

				FILE* file = fopen(fName, "wb");
				fwrite(&(this->header.fileHeader.bfType[0]), sizeof(uint8), 2, file);
				fwrite(&(this->header.fileHeader.bfSize), sizeof(uint32), 1, file);
				fwrite(&(this->header.fileHeader.bfReserved[0]), sizeof(uint16), 2, file);
				fwrite(&(this->header.fileHeader.bfOffBits), sizeof(uint32), 1, file);
				fwrite(&(this->header.infoHeader.biSize), sizeof(uint32), 1, file);
				fwrite(&(this->header.infoHeader.biWidth), sizeof(int32), 1, file);
				fwrite(&(this->header.infoHeader.biHeight), sizeof(int32), 1, file);
				fwrite(&(this->header.infoHeader.biPlanes), sizeof(uint16), 1, file);
				fwrite(&(this->header.infoHeader.biBitCount), sizeof(uint16), 1, file);
				fwrite(&(this->header.infoHeader.biCompression), sizeof(uint32), 1, file);
				fwrite(&(this->header.infoHeader.biSizeImage), sizeof(uint32), 1, file);
				fwrite(&(this->header.infoHeader.biXPelsPerMeter), sizeof(int32), 1, file);
				fwrite(&(this->header.infoHeader.biYPelsPerMeter), sizeof(int32), 1, file);
				fwrite(&(this->header.infoHeader.biClrUsed), sizeof(uint32), 1, file);
				fwrite(&(this->header.infoHeader.biClrImportant), sizeof(uint32), 1, file);

				fseek(file, this->header.fileHeader.bfOffBits, SEEK_SET);

				for (int h = 0; h < height; h++) {
					for (int w = 0; w < width; w++) {
						fwrite(&(pixels[h][w]), sizeof(uint8), 3, file);
					}
					fwrite(dummy, sizeof(uint8), numPadding, file);
				}
				fclose(file);
				cout << "BMP File Successfully Generated. < " << fName << " >" << endl;

				return 0;
			}
		}
	}

	// dummy.bmp를 이용한 헤더 생성 함수
	int set_header(const char* fName) {
		FILE* file = fopen(fName, "rb");
		fread(&(this->header.fileHeader.bfType[0]), sizeof(uint8), 2, file);
		fread(&(this->header.fileHeader.bfSize), sizeof(uint32), 1, file);
		fread(&(this->header.fileHeader.bfReserved[0]), sizeof(uint16), 2, file);
		fread(&(this->header.fileHeader.bfOffBits), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biSize), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biWidth), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biHeight), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biPlanes), sizeof(uint16), 1, file);
		fread(&(this->header.infoHeader.biBitCount), sizeof(uint16), 1, file);
		fread(&(this->header.infoHeader.biCompression), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biSizeImage), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biXPelsPerMeter), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biYPelsPerMeter), sizeof(int32), 1, file);
		fread(&(this->header.infoHeader.biClrUsed), sizeof(uint32), 1, file);
		fread(&(this->header.infoHeader.biClrImportant), sizeof(uint32), 1, file);
		fclose(file);

		int numPadding = (4 - ((this->width * 3) % 4)) % 4; // 4
		this->header.infoHeader.biWidth = this->width;
		this->header.infoHeader.biHeight = this->height;
		this->header.infoHeader.biSizeImage = (width * 3 + numPadding) * height;
		this->header.fileHeader.bfSize = this->header.infoHeader.biSizeImage + 54;

		return 0;
	}
};

// 지도 이미지에 색 입력 함수
void mark_location(BMPImage* image, int x, int y, int size, char color) {
	Pixel** pixels = image->pixels;
	// 색칠하려는 점들이 지도 안에 있을 때
	if (x - size > 0 && x + size < image->width && y - size > 0 && y + size < image->height) {
		// 입력받은 색과 크기에 따라 색칠
		if (color == 'b') {
			for (int i = y - size; i <= y + size; i++) {
				for (int j = x - size; j <= x + size; j++) {
					pixels[i][j].b = 255;
					pixels[i][j].g = 0;
					pixels[i][j].r = 0;
				}
			}
		}
		if (color == 'g') {
			for (int i = y - size; i <= y + size; i++) {
				for (int j = x - size; j <= x + size; j++) {
					pixels[i][j].b = 0;
					pixels[i][j].g = 255;
					pixels[i][j].r = 0;
				}
			}
		}
		if (color == 'r') {
			for (int i = y - size; i <= y + size; i++) {
				for (int j = x - size; j <= x + size; j++) {
					pixels[i][j].b = 0;
					pixels[i][j].g = 0;
					pixels[i][j].r = 255;
				}
			}
		}
	}
	return;
}


///// ///// ///// Point, Point_list ///// ///// /////

// Point struct
typedef struct {
	string name;
	string type;
	int x;
	int y;
} Point;

// 입력받은 이름의 파일에서 pointlist를 불러오는 함수
void load_Point_list(const char* Fname, Point* list, int* list_idx) {
	ifstream File;
	File.open(Fname);
	if (File.is_open())
	{
		while (!File.eof())
		{
			File >> list[*list_idx].type >> list[*list_idx].name >> list[*list_idx].x >> list[*list_idx].y;
			(*list_idx)++;
		}
		File.close();
	}
}

// pointlist를 출력하는 함수
void Print_Point_list(Point* list, int list_idx) {
	cout << endl << " -------------  Point List ------------- " << endl << endl;
	for (int i = 0; i < list_idx; i++) {
		cout << setw(10) << list[i].type << setw(20) << list[i].name << setw(6) << list[i].x << setw(6)<< list[i].y << endl;
	}
}


///// ///// ///// Node, Pathfinding Algorithm ///// ///// /////

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
	Node* getmother() {
		return this->mother;
	}

	//노드의 부모노드를 재설정 하는 함수
	void setmother(Node* new_mother) {
		this->mother = new_mother;
	}
};

//  노드 메모리 할당 함수
Node** create_node_2D(int height, int width)
{
	Node** arr = new Node * [height];
	for (int i = 0; i < height; i++) {
		arr[i] = new Node[width];
	}
	return arr;
}

// 두 노드의 내용을 바꾸는 함수
void swap(Node** a, Node** b) {
	Node* tmp = *a;
	*a = *b;
	*b = tmp;
}

// 최소 heap에 새 노드를 삽입하는 함수
void heap_push(Node** heap, Node* data, int* heap_idx) {
	int idx = ++ (* heap_idx);
	heap[idx] = data; // heap의 마지막 노드에 데이터 삽입
	// heap[idx]의 부모 노드는 heap[idx / 2]
	while ((idx > 1) && (heap[idx]->f < heap[idx / 2]->f)) {
		// 자식노드의 f값이 더 작을경우 부모와 자식을 swap
		swap(heap[idx], heap[idx / 2]);
		idx = idx / 2; // 노드 위치 이동하여 다시 비교
	}
}

// heap의 루트 노드(heap[1])를 추출하고 남은 노드들을 정렬하는 함수
Node* heap_pop(Node** heap, int* heap_idx) {
	Node* result = heap[1]; // 루트 노드 추출
	heap[1] = heap[(*heap_idx)--]; // 루트 노드 자리에 마지막 노드를 옮기고 heap_idx를 감소시켜 마지막 노드 삭제
	int Parent = 1;
	int Child;
	while (1)
	{
		Child = Parent * 2;
		// heap[Child] == 부모노드의 왼쪽 자식노드 포인터, heap[Child + 1] == 부모노드의 오른쪽 자식노드 포인터
		// *heap[Child] == 부모노드의 왼쪽 자식노드, *heap[Child + 1] == 부모노드의 오른쪽 자식노드
		if ((Child + 1 <= *heap_idx) && ((*heap[Child]).f > (*heap[Child + 1]).f)) {
			// 오른쪽 노드가 존재하고, 왼쪽 노드보다 f값이 작으면,
			Child++; // Child를 증분하여, heap[Child] == 부모노드의 오른쪽 자식노드
		}
		if (Child > *heap_idx || (*heap[Child]).f > (*heap[Parent]).f) {
			// 부모노드의 f가 더 작으면 swap할 필요 없으므로 break
			break;
		}
		swap(heap[Child], heap[Parent]); // heap[parent]가 heap[child]보다 작으므로 swap
		Parent = Child; // 노드위치 이동하여 다시 비교
	}
	return result;
}

// data가 닫힌 리스트(closed)에 있는지 확인하는 함수
bool search_closed(Node** closed, int closed_idx, Node* data) {
	for (int i = 0; i < closed_idx; i++) {
		if (closed[i] == data) {
			return 1;
		}
	}
	return 0;
}

// idx_node에 인접한 노드를 판단하여 heap에 추가하거나 정보를 수정하는 함수
void get_child(int child_x, int child_y, int move_cost, Node* idx_node, Node** Mapnode, BMPImage map,  Node** heap, int* heap_idx, Node** closed, int* closed_idx , int dest_x, int dest_y) {
	// 추가하려는 노드가 맵 안에 있고,
	if (child_x > 0 && child_x < map.width && child_y > 0 && child_y < map.height) {
		// 닫힌 리스트(closed)에 없고, 갈 수 있는 길일 경우,
		if (!search_closed(closed, *closed_idx, &Mapnode[child_y][child_x]) && map.pixels[child_y][child_x].b == 255) {
			bool search_result = 0;
			int heap_search_tmp = 0;
			for (int i = 1; i < *heap_idx + 1; i++) {
				if (heap[i] == &Mapnode[child_y][child_x]) {
					search_result = 1;// 열린 리스트(heap)에 있을 경우 1 반환
					heap_search_tmp = i;
				}
			}
			// 열린 리스트에 있는 노드이고, 기존의 g값보다 지금 노드 경로로 가는 g값이 작은 경우,
			if (search_result == 1 && (Mapnode[child_y][child_x].g > ((*idx_node).g + move_cost))) {
				// 노드의 mother노드, g, f 값 변경
				Mapnode[child_y][child_x].setmother(idx_node);
				Mapnode[child_y][child_x].g = (*idx_node).g + move_cost;
				Mapnode[child_y][child_x].f = Mapnode[child_y][child_x].g + Mapnode[child_y][child_x].h;
				// heap[heap_search_tmp]의 부모 노드는 heap[heap_search_tmp / 2]
				// f 값이 변했으므로 heap 재정렬
				while ((heap_search_tmp > 1) && (heap[heap_search_tmp]->f < heap[heap_search_tmp / 2]->f)) {
					swap(heap[heap_search_tmp], heap[heap_search_tmp / 2]); // 자식노드의 f값이 더 작을경우 부모와 자식을 swap
					heap_search_tmp = heap_search_tmp / 2; // 노드위치 이동하여 다시 비교
				}
			}
			// 열린 리스트에 없는 노드일 경우,
			if (search_result == 0) {
				// 노드에 정보를 입력하고 열린 리스트 (heap)에 노드 추가
				Mapnode[child_y][child_x].setmother(idx_node);
				Mapnode[child_y][child_x].g = (*idx_node).g + move_cost;
				Mapnode[child_y][child_x].h = 10 * (abs(dest_x - (child_x)) + abs(dest_y - (child_y)));
				Mapnode[child_y][child_x].f = Mapnode[child_y][child_x].g + Mapnode[child_y][child_x].h;
				heap_push(heap, &Mapnode[child_y][child_x], heap_idx);
			}
		}
	}
}

// Mapnode를 이용하여 목적지까지 길을 찾는 함수
int find_path(Node** Mapnode, BMPImage* map, int start_x, int start_y, int dest_x, int dest_y) {
	cout << "Start searching path ( " << start_x << ", " << start_y << " ) to ( " << dest_x << ", " << dest_y << " ) " << endl << endl;

	// 시작점 생성
	Mapnode[start_y][start_x].g = 0;
	Mapnode[start_y][start_x].h = 10 * (abs(dest_x - start_x) + abs(dest_y - start_y)); // 휴리스틱 거리 h는 Manhattan Distance 이용
	Mapnode[start_y][start_x].f = Mapnode[start_y][start_x].g + Mapnode[start_y][start_x].h;

	// 열린 리스트 우선 순위 큐를 array를 이용한 최소 힙을 이용하여 구현
	Node** heap = new Node * [100000];
	int heap_idx = 0; // heap_idx == heap의 노드 개수

	// 닫힌 리스트 array
	Node** closed = new Node * [100000];
	int closed_idx = 0;

	// 탐색할 노드 idx_node 시작점으로 초기화
	Node* idx_node = &Mapnode[start_y][start_x];

	// 자식 노드를 만들때 사용할 인수 선언
	int child_x;
	int child_y;

	// 목적지에 도달할때까지,
	while ((*idx_node).x != dest_x || (*idx_node).y != dest_y) {
		closed[++closed_idx] = idx_node; // 노드를 닫힌 리스트에 추가
		int idx_x = (*idx_node).x;
		int idx_y = (*idx_node).y;

		// idx_node에 인접한 노드 8개 판단
		
		// 수직방향 노드 4개 : 이동비용 10
		child_x = idx_x + 1;
		child_y = idx_y;
		get_child(child_x, child_y, 10, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		child_x = idx_x;
		child_y = idx_y + 1;
		get_child(child_x, child_y, 10, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		child_x = idx_x - 1;
		child_y = idx_y;
		get_child(child_x, child_y, 10, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		child_x = idx_x;
		child_y = idx_y - 1;
		get_child(child_x, child_y, 10, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		// 대각선 방향 노드 4개 : 이동비용 14
		child_x = idx_x + 1;
		child_y = idx_y + 1;
		get_child(child_x, child_y, 14, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		child_x = idx_x + 1;
		child_y = idx_y - 1;
		get_child(child_x, child_y, 14, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		child_x = idx_x - 1;
		child_y = idx_y - 1;
		get_child(child_x, child_y, 14, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);

		child_x = idx_x - 1;
		child_y = idx_y + 1;
		get_child(child_x, child_y, 14, idx_node, Mapnode, *map, heap, &heap_idx, closed, &closed_idx, dest_x, dest_y);
		
		// 갈 수 있는 모든 길을 시도해도 목적지를 갈 수 없을 때 종료
		if (heap_idx == 0) {
			cout << "No Answer !" << endl << "Searched " << heap_idx + closed_idx << " Nodes(pixels)" << endl << endl;;
			return 999999;
		}

		// 열린 리스트 (heap)에서 가장 f값이 작은 노드를 pop하여 idx_node 로 놓고 반복
		idx_node = heap_pop(heap, &heap_idx);
	}

	// while 문을 정상적으로 나왔다면 목적지까지 경로 찾기 성공
	cout << "Path found !" << endl;
	cout << "Searched " << heap_idx + closed_idx << " Nodes(pixels)" << endl;
	cout << "Path move cost : " << idx_node->g << endl << endl;

	// 출발지부터 목적지까지의 최단경로 cost
	int cost = idx_node->g;

	// 이미지에 표시, 경로 출력(optional)
	// cout << "Path :" ; // 경로 출력 (optional) ////////
	while (idx_node != NULL) {
		mark_location(map, (*idx_node).x, (*idx_node).y, 1, 'g');
		// cout << "- (" << (*idx_node).x << "," << (*idx_node).y << ")"; // 경로 출력 (optional) ////////
		idx_node = (*idx_node).getmother();
	}
	cout << endl;

	//cost 반환
	return cost;
}



///// ///// ///// main ///// ///// /////

int main() {
	BMPImage map; // BMPImage Class 선언
	// 지도 데이터 불러오기
	map.load_bmp_image("map.bmp");

	// 이차원 클래스 배열 선언 및 초기화
	Node** Mapnode;
	Mapnode = create_node_2D(map.height, map.width);
	for (int h = 0; h < map.height; h++) {
		for (int w = 0; w < map.width; w++) {
			Mapnode[h][w].x = w;
			Mapnode[h][w].y = h;
		}
	}

	// 출발점과 목적지를 담을 변수
	int start_x = 0;
	int start_y = 0;
	int dest_x = 0;
	int dest_y = 0;

	// 위치정보를 담을 point list에 메모리 할당
	int point_list_size = 0;
	Point* list = new Point[10000];

	// point list에 데이터 불러옴
	load_Point_list("point_list.txt", list, &point_list_size);
	Print_Point_list(list, point_list_size);

	// Point list에서 찾기 위해 사용할 변수 선언
	int start_idx=-1;
	int dest_idx=-1;
	string str_input;

	//현재 위치와 목적지 입력받음
	cout << endl << "현재 위치를 입력해 주세요 : ";
	cin >> str_input;
	for (int i = 0; i < point_list_size; i++) {
		if (str_input == list[i].name) {
			cout << "현재 위치 : " << list[i].name << endl << endl;
			start_x = list[i].x;
			start_y = list[i].y;
			start_idx = i;
		}
	}

	int ask_factor;
	cout << endl << "무엇을 하시겠습니까 ? " << endl;
	cout << endl << "1 정해진 목적지로의 경로 안내" << endl << "2 가장 가까운 카페, 음식점 또는 술집 탐색 " << endl << endl;
	cin >> ask_factor;
	// 선택1. 목적지를 입력하면 목적지까지의 경로를 찾고 지도에 나타낸다
	if (ask_factor == 1) {
		cout << "목적지를 입력해 주세요 : ";
		cin >> str_input;
		for (int i = 0; i < point_list_size; i++) {
			if (str_input == list[i].name) {
				cout << "목적지 : " << list[i].name << endl << endl;
				dest_x = list[i].x;
				dest_y = list[i].y;
				dest_idx = i;
			}
		}

		// 입력받은 두 point가 모두 list에 존재할경우 길찾기
		if (start_idx != -1 && dest_idx != -1) {
			cout << "\"" << list[start_idx].name << "\" 에서 \"" << list[dest_idx].name << "\" (으)로 가는 길을 찾습니다" << endl;
		}
		else {
			cout << "리스트에 없음 " << endl;
			// 아닐경우 종료
			return -1;
		}

		// 길 찾고 map 에 경로 표시
		find_path(Mapnode, &map, start_x, start_y, dest_x, dest_y);

		// 시작점 표시
		mark_location(&map, start_x, start_y, 2, 'r');

		// 목적지 표시
		mark_location(&map, dest_x, dest_y, 2, 'b');

		// 결과 이미지 파일로 저장
		map.save_bmp_image("Path_result.bmp");

		return 0;
	}
	// 선택2. 찾으려는 가게의 유형 중 직선거리가 가장 가까운 가게로 가는 경로를 찾고 지도에 나타낸다.
	if (ask_factor == 2) {
		cout << "찾으려는 가게의 유형을 입력해 주세요 : ";
		cin >> str_input;
		cout << endl;
		int idx = -1;
		int tmp = 999999;
		for (int i = 0; i < point_list_size; i++) {
			if (str_input == list[i].type) {
				if (tmp > abs(start_x - list[i].x) * abs(start_x - list[i].x) + abs(start_y - list[i].y) * abs(start_y - list[i].y)) {
					tmp = abs(start_x - list[i].x) * abs(start_x - list[i].x) + abs(start_y - list[i].y) * abs(start_y - list[i].y);
					idx = i;
				}
			}
		}
		// 길 찾고 map 에 경로 표시
		tmp = find_path(Mapnode, &map, start_x, start_y, list[idx].x, list[idx].y);
		if (tmp != 999999) {
			cout << "가장 가까운 " << str_input << "(은)는 \"" << list[idx].name << "\" 이고, 이동비용은 " << tmp << "입니다" << endl;
			// 시작점 표시
			mark_location(&map, start_x, start_y, 2, 'r');

			// 목적지 표시
			mark_location(&map, list[idx].x, list[idx].y, 2, 'b');

			// 결과 이미지 파일로 저장
			map.save_bmp_image("Path_result.bmp");

			return 0;
		}
		else {
			cout << "error" << endl;

			return 0;
		}
	}
	else {
		cout << "wrong input" << endl;
		return 0;
	}
}


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
	uint8 bfType[2];	//BMP���� �ĺ��� ���� Magic Number : 0x42, 0x4D
	uint32 bfSize;	//BMP���� ũ��
	uint16 bfReserved[2];	//�غ�. �׸��� �׸� ���� ���α׷��� ���� �ٸ�.
	uint32 bfOffBits;	//Offset. ��Ʈ�� �����͸� ã�� �� �ִ� ����	
} BMPFileHeader;
// BMPInfoHeader struct
typedef struct {
	uint32 biSize; //�� ����ü�� ũ��
	int32 biWidth; //����
	int32 biHeight; //����
	uint16 biPlanes; //����ϴ� color plane. �⺻ �� : 1
	uint16 biBitCount; //�� ȭ�ҿ� ���� ��Ʈ��. 8bit * RGB = 24
	uint32 biCompression;  //���� ���
	uint32 biSizeImage; //������� ���� ��Ʈ�� �������� ũ��
	int32 biXPelsPerMeter; //�׸��� �ػ�
	int32 biYPelsPerMeter; // �׸��� �ػ�
	uint32 biClrUsed; //�� �ȷ�Ʈ�� �� ��, �Ǵ� 0���� �⺻ �� 2^n
	uint32 biClrImportant; //�߿��� ���� ��. ���� ��� �߿��� ��� 0. �Ϲ������� ����.
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
// �ȼ� �޸� �Ҵ� �Լ�
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

	// �� �ȼ��� ����� �Լ�
	void create_empty_pixels(int height, int width){
		this->height = height;
		this->width = width;
		this->pixels = create_pixels_2D(height, width);
		set_header("dummy.bmp");
		cout << "create empty_pixel" << endl;
	}

	// �Է¹��� �̸��� ������ �ҷ����� �Լ�
	int load_bmp_image(const char* fName) {
		//���� pixels�� ������ ����ִٸ� memory release
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

		// ��Ʈ���� ���θ� 4�� ����� �����ϹǷ� ���� ������ ä�� padding�� ����
		int numPadding = (4 - ((width * 3) % 4)) % 4;
		uint8 dummy[4];

		// �ȼ� �������� ���� ��ġ�� ���������� �̵�
		fseek(file, this->header.fileHeader.bfOffBits, SEEK_SET);

		// pixels�� �ȼ� ������ ����
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

	// ������ �����ϴ� �Լ�
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

	// dummy.bmp�� �̿��� ��� ���� �Լ�
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

// ���� �̹����� �� �Է� �Լ�
void mark_location(BMPImage* image, int x, int y, int size, char color) {
	Pixel** pixels = image->pixels;
	// ��ĥ�Ϸ��� ������ ���� �ȿ� ���� ��
	if (x - size > 0 && x + size < image->width && y - size > 0 && y + size < image->height) {
		// �Է¹��� ���� ũ�⿡ ���� ��ĥ
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

// �Է¹��� �̸��� ���Ͽ��� pointlist�� �ҷ����� �Լ�
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

// pointlist�� ����ϴ� �Լ�
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

	// ������
	Node() {
		x = 0;
		y = 0;
		f = 0;
		g = 0;
		h = 0;
		this->mother = NULL;
	}

	~Node() {}

	//����� �θ��带 return �ϴ� �Լ�
	Node* getmother() {
		return this->mother;
	}

	//����� �θ��带 �缳�� �ϴ� �Լ�
	void setmother(Node* new_mother) {
		this->mother = new_mother;
	}
};

//  ��� �޸� �Ҵ� �Լ�
Node** create_node_2D(int height, int width)
{
	Node** arr = new Node * [height];
	for (int i = 0; i < height; i++) {
		arr[i] = new Node[width];
	}
	return arr;
}

// �� ����� ������ �ٲٴ� �Լ�
void swap(Node** a, Node** b) {
	Node* tmp = *a;
	*a = *b;
	*b = tmp;
}

// �ּ� heap�� �� ��带 �����ϴ� �Լ�
void heap_push(Node** heap, Node* data, int* heap_idx) {
	int idx = ++ (* heap_idx);
	heap[idx] = data; // heap�� ������ ��忡 ������ ����
	// heap[idx]�� �θ� ���� heap[idx / 2]
	while ((idx > 1) && (heap[idx]->f < heap[idx / 2]->f)) {
		// �ڽĳ���� f���� �� ������� �θ�� �ڽ��� swap
		swap(heap[idx], heap[idx / 2]);
		idx = idx / 2; // ��� ��ġ �̵��Ͽ� �ٽ� ��
	}
}

// heap�� ��Ʈ ���(heap[1])�� �����ϰ� ���� ������ �����ϴ� �Լ�
Node* heap_pop(Node** heap, int* heap_idx) {
	Node* result = heap[1]; // ��Ʈ ��� ����
	heap[1] = heap[(*heap_idx)--]; // ��Ʈ ��� �ڸ��� ������ ��带 �ű�� heap_idx�� ���ҽ��� ������ ��� ����
	int Parent = 1;
	int Child;
	while (1)
	{
		Child = Parent * 2;
		// heap[Child] == �θ����� ���� �ڽĳ�� ������, heap[Child + 1] == �θ����� ������ �ڽĳ�� ������
		// *heap[Child] == �θ����� ���� �ڽĳ��, *heap[Child + 1] == �θ����� ������ �ڽĳ��
		if ((Child + 1 <= *heap_idx) && ((*heap[Child]).f > (*heap[Child + 1]).f)) {
			// ������ ��尡 �����ϰ�, ���� ��庸�� f���� ������,
			Child++; // Child�� �����Ͽ�, heap[Child] == �θ����� ������ �ڽĳ��
		}
		if (Child > *heap_idx || (*heap[Child]).f > (*heap[Parent]).f) {
			// �θ����� f�� �� ������ swap�� �ʿ� �����Ƿ� break
			break;
		}
		swap(heap[Child], heap[Parent]); // heap[parent]�� heap[child]���� �����Ƿ� swap
		Parent = Child; // �����ġ �̵��Ͽ� �ٽ� ��
	}
	return result;
}

// data�� ���� ����Ʈ(closed)�� �ִ��� Ȯ���ϴ� �Լ�
bool search_closed(Node** closed, int closed_idx, Node* data) {
	for (int i = 0; i < closed_idx; i++) {
		if (closed[i] == data) {
			return 1;
		}
	}
	return 0;
}

// idx_node�� ������ ��带 �Ǵ��Ͽ� heap�� �߰��ϰų� ������ �����ϴ� �Լ�
void get_child(int child_x, int child_y, int move_cost, Node* idx_node, Node** Mapnode, BMPImage map,  Node** heap, int* heap_idx, Node** closed, int* closed_idx , int dest_x, int dest_y) {
	// �߰��Ϸ��� ��尡 �� �ȿ� �ְ�,
	if (child_x > 0 && child_x < map.width && child_y > 0 && child_y < map.height) {
		// ���� ����Ʈ(closed)�� ����, �� �� �ִ� ���� ���,
		if (!search_closed(closed, *closed_idx, &Mapnode[child_y][child_x]) && map.pixels[child_y][child_x].b == 255) {
			bool search_result = 0;
			int heap_search_tmp = 0;
			for (int i = 1; i < *heap_idx + 1; i++) {
				if (heap[i] == &Mapnode[child_y][child_x]) {
					search_result = 1;// ���� ����Ʈ(heap)�� ���� ��� 1 ��ȯ
					heap_search_tmp = i;
				}
			}
			// ���� ����Ʈ�� �ִ� ����̰�, ������ g������ ���� ��� ��η� ���� g���� ���� ���,
			if (search_result == 1 && (Mapnode[child_y][child_x].g > ((*idx_node).g + move_cost))) {
				// ����� mother���, g, f �� ����
				Mapnode[child_y][child_x].setmother(idx_node);
				Mapnode[child_y][child_x].g = (*idx_node).g + move_cost;
				Mapnode[child_y][child_x].f = Mapnode[child_y][child_x].g + Mapnode[child_y][child_x].h;
				// heap[heap_search_tmp]�� �θ� ���� heap[heap_search_tmp / 2]
				// f ���� �������Ƿ� heap ������
				while ((heap_search_tmp > 1) && (heap[heap_search_tmp]->f < heap[heap_search_tmp / 2]->f)) {
					swap(heap[heap_search_tmp], heap[heap_search_tmp / 2]); // �ڽĳ���� f���� �� ������� �θ�� �ڽ��� swap
					heap_search_tmp = heap_search_tmp / 2; // �����ġ �̵��Ͽ� �ٽ� ��
				}
			}
			// ���� ����Ʈ�� ���� ����� ���,
			if (search_result == 0) {
				// ��忡 ������ �Է��ϰ� ���� ����Ʈ (heap)�� ��� �߰�
				Mapnode[child_y][child_x].setmother(idx_node);
				Mapnode[child_y][child_x].g = (*idx_node).g + move_cost;
				Mapnode[child_y][child_x].h = 10 * (abs(dest_x - (child_x)) + abs(dest_y - (child_y)));
				Mapnode[child_y][child_x].f = Mapnode[child_y][child_x].g + Mapnode[child_y][child_x].h;
				heap_push(heap, &Mapnode[child_y][child_x], heap_idx);
			}
		}
	}
}

// Mapnode�� �̿��Ͽ� ���������� ���� ã�� �Լ�
int find_path(Node** Mapnode, BMPImage* map, int start_x, int start_y, int dest_x, int dest_y) {
	cout << "Start searching path ( " << start_x << ", " << start_y << " ) to ( " << dest_x << ", " << dest_y << " ) " << endl << endl;

	// ������ ����
	Mapnode[start_y][start_x].g = 0;
	Mapnode[start_y][start_x].h = 10 * (abs(dest_x - start_x) + abs(dest_y - start_y)); // �޸���ƽ �Ÿ� h�� Manhattan Distance �̿�
	Mapnode[start_y][start_x].f = Mapnode[start_y][start_x].g + Mapnode[start_y][start_x].h;

	// ���� ����Ʈ �켱 ���� ť�� array�� �̿��� �ּ� ���� �̿��Ͽ� ����
	Node** heap = new Node * [100000];
	int heap_idx = 0; // heap_idx == heap�� ��� ����

	// ���� ����Ʈ array
	Node** closed = new Node * [100000];
	int closed_idx = 0;

	// Ž���� ��� idx_node ���������� �ʱ�ȭ
	Node* idx_node = &Mapnode[start_y][start_x];

	// �ڽ� ��带 ���鶧 ����� �μ� ����
	int child_x;
	int child_y;

	// �������� �����Ҷ�����,
	while ((*idx_node).x != dest_x || (*idx_node).y != dest_y) {
		closed[++closed_idx] = idx_node; // ��带 ���� ����Ʈ�� �߰�
		int idx_x = (*idx_node).x;
		int idx_y = (*idx_node).y;

		// idx_node�� ������ ��� 8�� �Ǵ�
		
		// �������� ��� 4�� : �̵���� 10
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

		// �밢�� ���� ��� 4�� : �̵���� 14
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
		
		// �� �� �ִ� ��� ���� �õ��ص� �������� �� �� ���� �� ����
		if (heap_idx == 0) {
			cout << "No Answer !" << endl << "Searched " << heap_idx + closed_idx << " Nodes(pixels)" << endl << endl;;
			return 999999;
		}

		// ���� ����Ʈ (heap)���� ���� f���� ���� ��带 pop�Ͽ� idx_node �� ���� �ݺ�
		idx_node = heap_pop(heap, &heap_idx);
	}

	// while ���� ���������� ���Դٸ� ���������� ��� ã�� ����
	cout << "Path found !" << endl;
	cout << "Searched " << heap_idx + closed_idx << " Nodes(pixels)" << endl;
	cout << "Path move cost : " << idx_node->g << endl << endl;

	// ��������� ������������ �ִܰ�� cost
	int cost = idx_node->g;

	// �̹����� ǥ��, ��� ���(optional)
	// cout << "Path :" ; // ��� ��� (optional) ////////
	while (idx_node != NULL) {
		mark_location(map, (*idx_node).x, (*idx_node).y, 1, 'g');
		// cout << "- (" << (*idx_node).x << "," << (*idx_node).y << ")"; // ��� ��� (optional) ////////
		idx_node = (*idx_node).getmother();
	}
	cout << endl;

	//cost ��ȯ
	return cost;
}



///// ///// ///// main ///// ///// /////

int main() {
	BMPImage map; // BMPImage Class ����
	// ���� ������ �ҷ�����
	map.load_bmp_image("map.bmp");

	// ������ Ŭ���� �迭 ���� �� �ʱ�ȭ
	Node** Mapnode;
	Mapnode = create_node_2D(map.height, map.width);
	for (int h = 0; h < map.height; h++) {
		for (int w = 0; w < map.width; w++) {
			Mapnode[h][w].x = w;
			Mapnode[h][w].y = h;
		}
	}

	// ������� �������� ���� ����
	int start_x = 0;
	int start_y = 0;
	int dest_x = 0;
	int dest_y = 0;

	// ��ġ������ ���� point list�� �޸� �Ҵ�
	int point_list_size = 0;
	Point* list = new Point[10000];

	// point list�� ������ �ҷ���
	load_Point_list("point_list.txt", list, &point_list_size);
	Print_Point_list(list, point_list_size);

	// Point list���� ã�� ���� ����� ���� ����
	int start_idx=-1;
	int dest_idx=-1;
	string str_input;

	//���� ��ġ�� ������ �Է¹���
	cout << endl << "���� ��ġ�� �Է��� �ּ��� : ";
	cin >> str_input;
	for (int i = 0; i < point_list_size; i++) {
		if (str_input == list[i].name) {
			cout << "���� ��ġ : " << list[i].name << endl << endl;
			start_x = list[i].x;
			start_y = list[i].y;
			start_idx = i;
		}
	}

	int ask_factor;
	cout << endl << "������ �Ͻðڽ��ϱ� ? " << endl;
	cout << endl << "1 ������ ���������� ��� �ȳ�" << endl << "2 ���� ����� ī��, ������ �Ǵ� ���� Ž�� " << endl << endl;
	cin >> ask_factor;
	// ����1. �������� �Է��ϸ� ������������ ��θ� ã�� ������ ��Ÿ����
	if (ask_factor == 1) {
		cout << "�������� �Է��� �ּ��� : ";
		cin >> str_input;
		for (int i = 0; i < point_list_size; i++) {
			if (str_input == list[i].name) {
				cout << "������ : " << list[i].name << endl << endl;
				dest_x = list[i].x;
				dest_y = list[i].y;
				dest_idx = i;
			}
		}

		// �Է¹��� �� point�� ��� list�� �����Ұ�� ��ã��
		if (start_idx != -1 && dest_idx != -1) {
			cout << "\"" << list[start_idx].name << "\" ���� \"" << list[dest_idx].name << "\" (��)�� ���� ���� ã���ϴ�" << endl;
		}
		else {
			cout << "����Ʈ�� ���� " << endl;
			// �ƴҰ�� ����
			return -1;
		}

		// �� ã�� map �� ��� ǥ��
		find_path(Mapnode, &map, start_x, start_y, dest_x, dest_y);

		// ������ ǥ��
		mark_location(&map, start_x, start_y, 2, 'r');

		// ������ ǥ��
		mark_location(&map, dest_x, dest_y, 2, 'b');

		// ��� �̹��� ���Ϸ� ����
		map.save_bmp_image("Path_result.bmp");

		return 0;
	}
	// ����2. ã������ ������ ���� �� �����Ÿ��� ���� ����� ���Է� ���� ��θ� ã�� ������ ��Ÿ����.
	if (ask_factor == 2) {
		cout << "ã������ ������ ������ �Է��� �ּ��� : ";
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
		// �� ã�� map �� ��� ǥ��
		tmp = find_path(Mapnode, &map, start_x, start_y, list[idx].x, list[idx].y);
		if (tmp != 999999) {
			cout << "���� ����� " << str_input << "(��)�� \"" << list[idx].name << "\" �̰�, �̵������ " << tmp << "�Դϴ�" << endl;
			// ������ ǥ��
			mark_location(&map, start_x, start_y, 2, 'r');

			// ������ ǥ��
			mark_location(&map, list[idx].x, list[idx].y, 2, 'b');

			// ��� �̹��� ���Ϸ� ����
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


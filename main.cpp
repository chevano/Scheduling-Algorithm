#include <iostream>
#include <fstream>
#include <string>

using namespace std;


class node
{
	public:
		int jobId;
		int jobTime;

		// Constructor
		node(int, int);
};

node :: node(int id, int time)
{
	this->jobId = id;
	this->jobTime = time;
}


class qNode
{
	public:
		int jobId;
		int num_of_dpts;
		qNode* next;

		qNode();
		qNode(int, int);
};

// Dummy node
qNode :: qNode()
{
	this->jobId = 999;
	this->num_of_dpts = 9999;
	this->next = NULL;
}

qNode :: qNode(int id, int n)
{
	this->jobId = id;
	this->num_of_dpts = n;
	this->next = NULL;
}

class linkedList
{
	public:
		qNode* head;

		// Constructors
		linkedList();
		void insert2Open(qNode *);
		qNode * removal();
		void printList();
};

linkedList :: linkedList()
{
	this->head = new qNode();
}

void linkedList :: insert2Open(qNode *newNode)
{
	qNode* ptr = this->head;

	while(ptr->next != NULL)
	{
		if(newNode->num_of_dpts > ptr->next->num_of_dpts)
		{
			newNode->next = ptr->next;
			ptr->next = newNode;
			return;
		}
		else
			ptr = ptr->next;	// next step
	}
	newNode->next = ptr->next;
	ptr->next = newNode;
	return;
}

qNode* linkedList :: removal()
{
	qNode* temp = this->head->next;
	if(temp == NULL)
		return NULL;
	else
	{
		this->head = this->head->next;
	}
	return temp;
}

void linkedList :: printList()
{
	qNode* ptr = new qNode();
	ptr = this->head->next;
	cout << "OPEN -> ";
	while(ptr != NULL)
	{
		cout << ptr->jobId << " -> ";
		ptr = ptr->next;
	}
	cout << "NULL" << endl;
	return;
}

class Scheduling
{
	public:
		int numNodes;			// Total number of nodes(jobs)
		int totalJobTimes;		// Total time of all jobs
		int procGiven;			// Given number of processors
		int** adjacencyMatrix;	// 2-D matrix recording the edges
		int** scheduleTable;	// The schedule table what should be worked on
		int* jobTimeAry;		// Recording job time for each job
		linkedList* OPEN;		// OPEN Queue
		int* processJob;		// Recording the current job the i th processor working on
		int* processTime;		// Recording the current job's remaining time
		int* parentCount;		// The number of parent nodes with node i
		int* kidCount;			// The number of kid nodes with node i
		int* jobDone;			// Mark for done job, 0 == not done, 1 == done
		int* jobMarked;			// Mark for graph G, 0 == job is not marked, 1 == job is marked 

		// Constructor
		Scheduling();

		// Methods
		void loadMatrix(string);			// Loading data from file
											// and initializing the size of all dyn-ary
		void computeTotalJobTimes(string);	// Finding total job time of all job
											// and update job time array
		int getUnMarkOrphen();				// Find the next orphen, just 1 orphen
		string printTable(int);				// Printing the schedule table to file
		int findProcessor();				// Finding the next available processor, return -1 if there's no one
		void updateTable(int, int, int);	// Updating talbe with parameters (Proc id, job id, time slice)
		int checkCycle();					// Checking if the graph has cycle, 1 == yes, 0 == no
		int findDoneJob();					// Finding if a job is done by processr i, return job id if there is
		void deleteNode(int);				// Deleting the done job(id = i) node
		void deleteEdge(int);				// Deleting the edges pointed out from done job(id = i) node
		bool allDone();						// Return true if all jobs are done
};

void print_append(string content, string input_file_name)
{
	ofstream outFile(input_file_name, fstream::app);
	outFile << content;
	outFile.close();
}

void file_cleaner(string file_name)
{
	ofstream outFile(file_name, fstream::trunc);
	outFile.close();
}

Scheduling :: Scheduling()
{
	this->numNodes = 0;
	this->totalJobTimes = 0;
	this->procGiven = 0;
	this->OPEN = new linkedList();
}

void Scheduling :: loadMatrix(string input_file_name)
{
	ifstream inFile;
	inFile.open(input_file_name);
	
	int numOfJobs;
	int thisJob;
	int nextJob;
	string word;

	inFile >> word;
	numOfJobs = atoi(word.c_str());	// Get the number of nodes
	this->numNodes = numOfJobs;		// Seg FAULT!!!!!!

	// Initializing every dynamic array in this class except schedule table(need totalJobTimes)
	this->adjacencyMatrix = new int*[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		this->adjacencyMatrix[i] = new int[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		for(int j = 0; j <= numOfJobs; j++)
			this->adjacencyMatrix[i][j] = 0;

	this->scheduleTable = new int*[numOfJobs + 1];	// Column need to be defined

	this->jobTimeAry = new int[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		this->jobTimeAry[i] = 0;

	this->parentCount = new int[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		this->parentCount[i] = 0;

	this->kidCount = new int[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		this->kidCount[i] = 0;

	this->jobDone = new int[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		this->jobDone[i] = 0;

	this->jobMarked = new int[numOfJobs + 1];
	for(int i = 0; i <= numOfJobs; i++)
		this->jobMarked[i] = 0;

	// Inputing the matrix
	int r;
	int c;

	while(inFile >> word)
	{
		r = atoi(word.c_str());
		inFile >> word;
		c = atoi(word.c_str());
		this->adjacencyMatrix[r][c] = 1;
	}

	// Finding parentCount and kidCount
	int parent_id;
	int kid_id;
	for(parent_id = 1; parent_id <= numOfJobs; parent_id++)
	{
		for(int i = 1; i <= numOfJobs; i++)
			if(this->adjacencyMatrix[parent_id][i] == 1)
				this->kidCount[parent_id]++;
	}
	for(kid_id = 1; kid_id <= numOfJobs; kid_id++)
	{
		for(int i = 1; i <= numOfJobs; i++)
			if(this->adjacencyMatrix[i][kid_id] == 1)
				this->parentCount[kid_id]++;
	}

	inFile.close();
	return;
}

void Scheduling :: computeTotalJobTimes(string input_file_name)
{
	ifstream inFile;
	inFile.open(input_file_name);

	int total = 0;
	string word;
	inFile >> word;
	int numOfJobs = atoi(word.c_str());
	if(numOfJobs != this->numNodes)
	{
		cout << "Number of Nodes does not match in two files. ERROR!" << endl;
		exit(0);
	}
	int j_id;
	int j_time;
	while(inFile >> word)
	{
		j_id = atoi(word.c_str());
		inFile >> word;
		j_time = atoi(word.c_str());
		this->jobTimeAry[j_id] = j_time;
		total += j_time;
	}

	this->totalJobTimes = total;

	for(int i = 0; i <= numOfJobs; i++)
		this->scheduleTable[i] = new int[total + 1];

	inFile.close();
	return;
}

int Scheduling :: getUnMarkOrphen()
{
	for(int i = 1; i <= this->numNodes; i++)
	{
		if(this->jobMarked[i] == 0 && this->parentCount[i] == 0)
		{
			this->jobMarked[i] = 1;
			return i;
		}
	}
	return -1;
}

int Scheduling :: findProcessor()
{
	for(int i = 1; i <= this->procGiven; i++)
		if(this->processTime[i] == 0)
			return i;
	return -1;
}

void Scheduling :: updateTable(int availProc, int newJob, int currentTime)
{
	int j_time = this->jobTimeAry[newJob];
	int end = currentTime + j_time;
	for(int i = currentTime; i <= end; i++)
		this->scheduleTable[availProc][i] = newJob;

	return;
}

int Scheduling :: checkCycle()
{
	bool OPEN_is_empty = false;
	bool Graph_is_not_empty = false;
	bool Proc_all_finished = true;

	if(this->OPEN->head->next == NULL)
		OPEN_is_empty = true;
	
	for(int i = 1; i <= this->numNodes; i++)
		if(this->jobDone[i] == 0)
			Graph_is_not_empty = true;

	for(int i = 1; i <= this->procGiven; i++)
		if(this->processJob[i] != 0)
			Proc_all_finished = false;

	if(OPEN_is_empty && Graph_is_not_empty && Proc_all_finished)
		return 1;
	else
		return 0;
}

bool Scheduling :: allDone()
{
	bool done = true;

	for(int i = 1; i <= this->numNodes; i++)
		if(this->jobDone[i] != 1)
			done = false;

	return done;
}

// Return 0 if there's no job is done
int Scheduling :: findDoneJob()
{
	int done_job = 0;

	for(int i = 1; i <= this->procGiven; i++)
		if(this->processTime[i] == 0 && this->processJob[i] != 0)
		{
			done_job = this->processJob[i];
			this->processJob[i] = 0;
			return done_job;
		}
	
	return done_job;
}

void Scheduling :: deleteNode(int j_id)
{
	this->jobDone[j_id] = 1;
	
	return;
}

void Scheduling :: deleteEdge(int j_id)
{
	for(int i = 1; i <= this->numNodes; i++)
		if(this->adjacencyMatrix[j_id][i] != 0)
			this->parentCount[i]--;
	
	return;
}

string Scheduling :: printTable(int timeLine)
{
	string line[this->procGiven + 1];
	line[0] = "      ";
	for(int i = 1; i <= this->procGiven; i++)
		line[i] = "P(" + std::to_string(i) + ") |";

	for(int i = 0; i < timeLine; i++)
		line[0] += "--" + std::to_string(i) + "--\t";

	for(int i = 1; i <= this->procGiven; i++)
		for(int j = 0; j < timeLine; j++)
		{
			if(this->scheduleTable[i][j] == 0)
				line[i] += "  -  |\t";
			else if(this->scheduleTable[i][j] < 10)
				line[i] += "  " + std::to_string(this->scheduleTable[i][j]) + "  |\t";
			else
				line[i] += "  " + std::to_string(this->scheduleTable[i][j]) + " |\t";
		}

	string seperator = "      ";
	for(int i = 0; i < timeLine; i++)
		seperator += "--------";
	seperator += "\r\n";

	string result = "";
	for(int i = 0; i <= this->procGiven; i++)
	{
		result += line[i];
		result += "\r\n";
		if(i >= 1)
			result += seperator;
	}
	result += "\r\n\r\n";

	return result;
}



int main(int argc, char ** argv)
{
	Scheduling* s = new Scheduling();
	s->loadMatrix(argv[1]);
	s->computeTotalJobTimes(argv[2]);
	file_cleaner(argv[3]);
	file_cleaner(argv[4]);

	cout << "Please input the number of processors: ";
	cin >> s->procGiven;
	if(s->procGiven <= 0)
	{
		cout << "Illigal input!" << endl;
		exit(0);
	}
	else if(s->procGiven > s->numNodes)
		s->procGiven = s->numNodes;
	// Allocating processJob and processTime
	s->processJob = new int[s->procGiven + 1];
	s->processTime = new int[s->procGiven + 1];
	for(int i = 0; i <= s->procGiven; i++)
	{
		s->processJob[i] = 0;
		s->processTime[i] = 0;
	}

	int procUsed = 0;
	int currentTime = 0;
	int availProc = -1;

	while(!s->allDone())
	{
		int orphen_id = s->getUnMarkOrphen();
		while(orphen_id != -1)
		{
			int orphen_job_time = s->jobTimeAry[orphen_id];
			int orphen_kid_num = s->kidCount[orphen_id];

			qNode* temp_qNode = new qNode(orphen_id, orphen_kid_num);
			s->OPEN->insert2Open(temp_qNode);
		
			orphen_id = s->getUnMarkOrphen();
		}

		s->OPEN->printList();

		qNode* pop_Open_qNode = new qNode();
		if(procUsed < s->procGiven)
			pop_Open_qNode = s->OPEN->removal();

		while(pop_Open_qNode != NULL and procUsed < s->procGiven)
		{
			availProc = s->findProcessor();
			if(availProc > 0)
			{
				procUsed++;
				int newJob_id = pop_Open_qNode->jobId;
				int newJob_time = s->jobTimeAry[newJob_id];
				s->processJob[availProc] = newJob_id;
				s->processTime[availProc] = newJob_time;
				s->updateTable(availProc, newJob_id, currentTime);
			}
			if(procUsed < s->procGiven)
				pop_Open_qNode = s->OPEN->removal();
		}

		int cyc = s->checkCycle();
		if(cyc == 1)	// Cycle exist
		{
			cout << "There is cycle in the graph" << endl;
			exit(0);
		}

		// printTable();
		print_append(s->printTable(currentTime), argv[3]);

		currentTime++;

		// Decreasing processTime by 1
		for(int i = 1; i <= s->procGiven; i++)
			if(s->processTime[i] > 0)
				s->processTime[i]--;
		
		int done_job_id = s->findDoneJob();
		while(done_job_id != 0)
		{
			s->deleteNode(done_job_id);
			s->deleteEdge(done_job_id);
			procUsed--;

			done_job_id = s->findDoneJob();
		}
	
		// Debugging
		string word2 = "";
		word2 += std::to_string(currentTime);
		word2 += "\r\n";
		for(int i = 1; i <= s->numNodes; i++)
			word2 += std::to_string(s->jobMarked[i]) + " ";
		word2 += "\r\n";
		for(int i = 1; i <= s->procGiven; i++)
			word2 += std::to_string(s->processTime[i]) + " ";
		word2 += "\r\n";
		for(int i = 1; i <= s->procGiven; i++)
			word2 += std::to_string( s->processJob[i]) + " ";
		word2 += "\r\n";
		for(int i = 1; i <= s->numNodes; i++)
			word2 += std::to_string(s->jobDone[i]) + " ";
		word2 += "\r\n";
		word2 += "\r\n\r\n";
	
		print_append(word2, argv[4]);	
	}

	print_append(s->printTable(currentTime), argv[3]);

	return 0;
}

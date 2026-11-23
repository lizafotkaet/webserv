using namespace std;


int main(){

	vector<int> ass{1,2,3,4};
	auto it{ass.begin()};

	for (const auto& hui : ass){
		cout << hui << '\n';	
	}

}
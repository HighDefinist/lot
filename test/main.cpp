#define useCPU
#include "mz/lot.h"
using namespace std;
using namespace std::mz;
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

TEST_CASE("lot", "Various tests of the functionality of 'lot'") {
  lot<int> A;
  SECTION("Add one element") {
    A.push_back(3);
    REQUIRE(A[0]==3);
    REQUIRE(A.size()==1);
    REQUIRE(A.capacity()>=1);
  }

  SECTION("Emptiness") {
    REQUIRE(A.size()==0);
    REQUIRE(A.capacity() == 0);
    A.Add(2);
    A.clear();
    REQUIRE(A.size()==0);
  }

  SECTION("Manual reserve") {
    A.reserve(5);
    REQUIRE(A.capacity() == 5);
    A.reserve(10);
    REQUIRE(A.capacity() == 10);
    A.reserve(5);
    REQUIRE(A.capacity() == 10);
    A.reserve(5,true);
    REQUIRE(A.capacity() == 5);
    A.reserve(10);
    REQUIRE(A.capacity() == 10);
    A.Add(3);
    REQUIRE(A.capacity() == 10);
    A.shrink_to_fit();
    REQUIRE(A.capacity() == 1);
    A.Free();
    REQUIRE(A.capacity()==0);
  }

  SECTION("Copy and Move") {
    lot<int> B = { 1,2,3,4 };
    A = B;
    REQUIRE(A[3]==B[2]+1);
    B.Take(A);
    REQUIRE(A.size()==0);
    REQUIRE(B.size()==8);
    A = move(B);
    REQUIRE(A.size()==8);
    REQUIRE(B.size()==0);
    REQUIRE(A[3]==A[6]+1);
    lot<int> C(A);
    REQUIRE(C[2]==C[5]+1);
  }

  SECTION("Various Constructions, Insertions, Deletions and accesses") {
    auto B = lot<int>{ 1,3,4 };
    auto C(B);
    auto D = move(C);
    REQUIRE(B.size() == 3);
    REQUIRE(C.size() == 0);
    REQUIRE(D.size() == 3);
    REQUIRE(D.data()[0] == 1);
    REQUIRE(B.front() == 1);
    REQUIRE(D.back() == 4);
    REQUIRE(D.at(1) == 3);
    REQUIRE_THROWS_AS(D.at(3), out_of_range);
    D.push_back(7);
    REQUIRE(D.at(3) == 7);
    D.pop_back();
    REQUIRE_THROWS_AS(D.at(3), out_of_range);
    D.AddEmpty();
    REQUIRE(D.size() == 4);
    D.Add(B);
    REQUIRE(D.size() == 7);
    D.Add(8, 6, 5);
    REQUIRE(D.size() == 10);
  }
}

TEST_CASE("lots_malloc","Various tests of the functionality of 'lots', using adapter_malloc") {
  lots<adapter_malloc<int>,int> A;
  lots<adapter_malloc<int>,int> B = {3,4,5};
  bool onCPU=true;

  SECTION("Inherited functionality test") {
    A.Add(3);
    REQUIRE(A[0]==3);
  }
  SECTION("Copy to device test") {
    A.Add(2);
    A.Add(4);
    REQUIRE(A.gDevCapacity()==0);
    A.DevFromHost();
    REQUIRE(A.gDevCapacity()==A.capacity());
    REQUIRE(A.DevIsInit());
    auto AdevP=A.DevGet();
    if(onCPU) REQUIRE(AdevP[1]==4);     
    A[1]=3;
    REQUIRE(A[1]==3);
    if(onCPU) REQUIRE(AdevP[1]==4);     
    A.HostFromDev();
    REQUIRE(A[1]==4);
    A.DevFree();
    REQUIRE(!A.DevIsInit());
  }
  SECTION("Partial copy and resize test") {
    A={2,4,5};
    A.DevFromHost();
    A[0]=1;
    A[1]=3;
    A.DevFromHost(1);
    A.HostFromDev();
    REQUIRE(A[0]==1);
    REQUIRE(A[1]==4);
    REQUIRE(A[2]==5);
    A[0]=-1;
    A[1]=-2;
    A[2]=-3;
    A.HostFromDev(1);
    REQUIRE(A[0]==1);
    REQUIRE(A[1]==-2);
    REQUIRE(A[2]==-3);
    A.HostFromDev(2,1);
    REQUIRE(A[2]==5);
    A[2]=-4;
    A.DevFromHost(2,1);
    A.HostFromDev();
    REQUIRE(A[0]==1);
    REQUIRE(A[1]==4);
    REQUIRE(A[2]==-4);
    A={1,3,4,5};
    A.DevFromHost();
    A[1]=6;
    A.resize(1);
    A[0]=2;
    A.HostFromDev();
    REQUIRE(A[0]==1);
    REQUIRE(A.size()==1);
    A.resize(2);
    REQUIRE(A[1]==6);
    A.HostFromDev();
    REQUIRE(A[1]==3);
  }
  SECTION("Invalid parameter test") {
    A={1,-2};
    REQUIRE_THROWS_AS(A.HostFromDev(),pu_runtime_error);   
  }
  //SECTION("Force size") {
  //  REQUIRE(!A.DevIsInit());
  //  A.DevForceSize(10);
  //  REQUIRE(A.size()==0);
  //  REQUIRE(A.DevIsInit());
  //}
}

TEST_CASE("lots_lot", "Various tests of the functionality of 'lots', using adapter_lot") {
  lots<adapter_lot<int>, int> A;
  lots<adapter_lot<int>, int> B = { 3,4,5 };
  bool onCPU = true;

  SECTION("Inherited functionality test") {
    A.Add(3);
    REQUIRE(A[0] == 3);
  }
  SECTION("Copy to device test") {
    A.Add(2);
    A.Add(4);
    REQUIRE(A.gDevCapacity() == 0);
    A.DevFromHost();
    REQUIRE(A.gDevCapacity() == A.capacity());
    REQUIRE(A.DevIsInit());
    auto AdevP = A.DevGet();
    if (onCPU) REQUIRE(AdevP[1] == 4);
    A[1] = 3;
    REQUIRE(A[1] == 3);
    if (onCPU) REQUIRE(AdevP[1] == 4);
    A.HostFromDev();
    REQUIRE(A[1] == 4);
    A.DevFree();
    REQUIRE(!A.DevIsInit());
  }
  SECTION("Partial copy and resize test") {
    A = { 2,4,5 };
    A.DevFromHost();
    A[0] = 1;
    A[1] = 3;
    A.DevFromHost(1);
    A.HostFromDev();
    REQUIRE(A[0] == 1);
    REQUIRE(A[1] == 4);
    REQUIRE(A[2] == 5);
    A[0] = -1;
    A[1] = -2;
    A[2] = -3;
    A.HostFromDev(1);
    REQUIRE(A[0] == 1);
    REQUIRE(A[1] == -2);
    REQUIRE(A[2] == -3);
    A.HostFromDev(2, 1);
    REQUIRE(A[2] == 5);
    A[2] = -4;
    A.DevFromHost(2, 1);
    A.HostFromDev();
    REQUIRE(A[0] == 1);
    REQUIRE(A[1] == 4);
    REQUIRE(A[2] == -4);
    A = { 1,3,4,5 };
    A.DevFromHost();
    A[1] = 6;
    A.resize(1);
    A[0] = 2;
    A.HostFromDev();
    REQUIRE(A[0] == 1);
    REQUIRE(A.size() == 1);
    A.resize(2);
    REQUIRE(A[1] == 6);
    A.HostFromDev();
    REQUIRE(A[1] == 3);
  }
  SECTION("Invalid parameter test") {
    A = { 1,-2 };
    REQUIRE_THROWS_AS(A.HostFromDev(), pu_runtime_error);
  }
  //SECTION("Force size") {
  //  REQUIRE(!A.DevIsInit());
  //  A.DevForceSize(10);
  //  REQUIRE(A.size()==0);
  //  REQUIRE(A.DevIsInit());
  //}
}


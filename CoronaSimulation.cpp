#include <iostream>
#include <vector>
#include <queue>

using namespace std;

enum Vaccine {
    astra,
    biontech,
    moderna,
    none
};

ostream& operator<<(ostream& output, Vaccine vaccine){
    if(vaccine == Vaccine::astra){
        output << "astra";
    } else if(vaccine == Vaccine::biontech){
        output <<"biontech";
    } else if(vaccine == Vaccine::moderna){
        output<<"moderna";
    } else {
        output<<"none";
    }
    return output;
}

class SimError : public std::exception{
public:
    string reason;
    SimError(const std::string& _reason) : reason(_reason){}
};


int no = 3093764;
class Patient {
public:
    Patient(bool _briefed = false, Vaccine _vaccine = Vaccine::none) 
    : id(no++), briefed(_briefed), vaccine(_vaccine){
    }
    
    int get_id(){ return id; }

    bool is_briefed() { return briefed; }

    void switch_briefed() { briefed ^= true; }

    void set_vaccinated(Vaccine _vaccine){
        vaccine = _vaccine;
    }

    friend ostream& operator<<(ostream& os, const Patient &patient){
        os<<"patient "<<to_string(patient.id);
        if(patient.vaccine != Vaccine::none){
            os<<" vaccinated with "<<patient.vaccine;
        } else if(patient.briefed){
            os<<" briefed";
        }
        return os;
    }

private:
    const int id;
    bool briefed;
    Vaccine vaccine;
};

class Station{
public:
    Station(string _station, Patient* _patient) : station(_station), patient(_patient){}
    virtual ~Station() {}
    virtual void enter(Patient *_patient) = 0;
    virtual Patient* leave() = 0;
    virtual void print() = 0;
protected:
    string station;
    Patient *patient;
};


class WaitingArea : public Station{
public:
    WaitingArea(string _station) : Station(_station, nullptr), count(0), waiting(nullptr) {}

    void enter(Patient *_patient){
        if(waiting == nullptr){
            waiting = new Waiting(_patient, nullptr);
        } else {
            Waiting* cursor = waiting;
            while(cursor->next != nullptr){
                cursor = cursor->next;
            }
            cursor->next = new Waiting(_patient, nullptr);
        }
        count++;
    }

    Patient* leave(){
        if(waiting == nullptr){
            string message = "no patient waiting in " + station;
            throw SimError(message);
        }
        Patient* ret = waiting->patient;
        waiting = waiting->next;
        count--;
        return ret;
    }

    void print(){
        cout<<"Waiting: "<<count<<endl;
        if(count > 0){
            cout<<"waiting patients:";
            Waiting *cursor = waiting;
            while(cursor != nullptr){
                cout<<" "<<cursor->patient->get_id();
                cursor = cursor->next;
            }
            cout<<endl;
        }
    }

private:
    class Waiting {
    public:    
        Patient *patient;
        Waiting *next;
        Waiting(Patient* _patient, Waiting* _next){
            patient = _patient;
            next = _next;
        }
    };
    int count;
    Waiting* waiting;

};

class Single : public Station{
public:
    Single(string station) : Station(station, nullptr) {
        total = 0;
    }
    int get_total() { return total; }
    void enter(Patient *_patient){
        patient = _patient;
        cout<<patient->get_id()<<" enters "<<station<<endl;
        total++;
        patient->switch_briefed();
    }
    Patient* leave() {
        cout<<patient<<" leave "<<station<<endl;
        return patient;
    }
    virtual void print() = 0;
private:
    int total;
};

class Administrative : public Single {
public:
    Administrative(string station) : Single(station) {}
    
    void print() {
        cout<<get_total()<<" patients processed"<<endl;
    }
};

class Medical : public Single{
public:
    Medical(string station, string _doctor) : Single(station), doctor(_doctor) {}
    void enter(Patient *patient){
        if(!patient->is_briefed()){
            throw SimError("not briefed patient can not be vaccinated ");
        }
        Single::enter(patient);
        patient->set_vaccinated(Vaccine::astra);
    }

    Patient* leave() {
        return Single::leave();
    }

    void print() {
        cout<<get_total()<<" patients vaccinated by "<<doctor<<endl;
    }
private:
    string doctor;
};

class Center{
public:
    Center() {
        station[0] = new Administrative("Registration and Briefing");
        station[1] = new WaitingArea("Wait for Vaccination");
        station[2] = new Medical("Vaccination", "Amr Abdou");
        station[3] = new WaitingArea("Wait after Vaccination");
        station[4] = new Administrative("Debriefing");
    }

    void brief(Patient *patient){
        station[0]->enter(patient);
        station[1]->enter(station[0]->leave());
    }

    void brief(){
        station[2]->enter(station[1]->leave());
        station[3]->enter(station[2]->leave());
    }

    void debrief(){
        station[4]->enter(station[3]->leave());
        delete station[4]->leave();
    }

    void print(){
        cout<<"LINE DATA"<<endl;
        for(int i = 0; i < 5; i++){
            station[i]->print();
        }
    }
private:
    Station* station[5];
};

int main(){
    Center c;
    char choice;
    do{
        try{
            c.print();
            cout<<"a. end"<<endl;
            cout<<"b. new arrival at center"<<endl;
            cout<<"c. next vaccination"<<endl;
            cout<<"d. next debriefing"<<endl;
            cin>>choice;
            if(choice == 'b'){
                Patient* patient = new Patient();
                c.brief(patient);
            } else if(choice == 'b'){
                c.brief();
            } else {
                c.debrief();
            }
        }catch(SimError &e){
            cout<<e.reason<<endl;
        }
    }while(choice != 'a');
    return 0;
}

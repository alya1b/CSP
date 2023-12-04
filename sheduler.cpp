//Аліса Баклан
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <functional>
#include <set>

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

class Teacher;
class Course;

constexpr int TIME_SLOTS = 5*4;

class Group {
public:
    int id;
    std::string name;
    std::vector<Course> courses; 

    Group(int _id, const std::string& _name, const std::vector<Course>& _courses)
        : id(_id), name(_name), courses(_courses) {}
};

class Course {
public:
    int id;
    std::string name;
    int lectureHours;
    int labHours;
    std::vector<Teacher> qualifiedTeachers; 

    Course(int _id, const std::string& _name, int _lectureHours, int _labHours, const std::vector<Teacher>& _qualifiedTeachers)
        : id(_id), name(_name), lectureHours(_lectureHours), labHours(_labHours), qualifiedTeachers(_qualifiedTeachers) {}
};

class Teacher {
public:
    int id;
    std::string name;
    int maxHours;
    std::unordered_map<int, int> assignedHours; 
    std::unordered_map<int, std::set<int>> lectureAssignments; 

    Teacher(int _id, std::string _name, int _maxHours) 
        : id(_id), name(_name), maxHours(_maxHours) {}
};

class Data {
public:
    std::vector<Teacher> teachers;
    std::vector<Group> groups;
    std::vector<Course> courses;

  Data() {

    teachers.push_back(Teacher(0, "Teacher1", 20)); // ID, Name, Max Hours
    teachers.push_back(Teacher(1, "Teacher2", 20));
    teachers.push_back(Teacher(2, "Teacher3", 20));
    teachers.push_back(Teacher(3, "Teacher4", 20));
    teachers.push_back(Teacher(4, "Teacher5", 20));
    teachers.push_back(Teacher(5, "Teacher6", 20));


    Course course1(0, "Course1", 4, 2, {teachers[2], teachers[1]});//id, name, lectures_hours, labs_hours, qualified teachers
    Course course2(1, "Course2", 1, 1, {teachers[0], teachers[1]});
    Course course3(2, "Course3", 2, 2, {teachers[4]});
    Course course4(3, "Course4", 0, 1, {teachers[0]});
    Course course5(4, "Course5", 3, 0, {teachers[4], teachers[2]});
    Course course6(5, "Course6", 1, 1, {teachers[0], teachers[3]});
    Course course7(6, "Course7", 1, 2, {teachers[0], teachers[5]});
    Course course8(7, "Course8", 2, 0, {teachers[3], teachers[4]});
    Course course9(8, "Course9", 0 , 1, {teachers[0], teachers[5]});
    Course course10(9, "Course10", 2, 2, {teachers[3], teachers[5]});

    courses.push_back(course1);
    courses.push_back(course2);
    courses.push_back(course3);
    courses.push_back(course4);
    courses.push_back(course5);
    courses.push_back(course6);
    courses.push_back(course7);
    courses.push_back(course8);
    courses.push_back(course9);
    courses.push_back(course10);

    std::vector<Course> group1Courses = {course1, course2};
    Group group1(0, "Group1", group1Courses);

    std::vector<Course> group2Courses = {course1, course3, course4,course5, course6};
    Group group2(1, "Group2", group2Courses);

    std::vector<Course> group3Courses = {course1, course2, course3, course10};
    Group group3(2, "Group3", group3Courses);

    std::vector<Course> group4Courses = {course5, course6, course7, course8, course10};
    Group group4(3, "Group4", group4Courses);

    groups.push_back(group1);
    groups.push_back(group2);
    groups.push_back(group3);
    groups.push_back(group4);
}

};


class Variable {
public:
    int id;
    int groupID;
    int courseId; 
    bool isLecture;
    int timeSlot; 
    int teacherID; 
    bool isAssigned; 

    Variable(int _id, int _groupID, int _subjectID, bool _isLecture)
        : id(_id), groupID(_groupID), courseId(_subjectID), isLecture(_isLecture),
          timeSlot(-1), teacherID(-1), isAssigned(false) {} 
 
};

class DomainSlot {
public:
    int teacherID;
    int timeSlot;

    DomainSlot(){
        teacherID = -1;
        timeSlot = -1;
    }

    DomainSlot(int _teacherID, int _timeSlot) : teacherID(_teacherID), timeSlot(_timeSlot) {}
};

class Domain {
public:
    int id; // ID corresponding to the variable
    std::vector<DomainSlot> domainSlots;
    int domainSlotsLeft; // Remaining domain slots for MRV

    Domain(int _id, const std::vector<DomainSlot>& slots)
        : id(_id), domainSlots(slots), domainSlotsLeft(slots.size()) {} 

    void updateDomainSlotsLeft() {
        domainSlotsLeft = domainSlots.size(); 
    }
};

void printVariables(const std::vector<Variable>& variables) {
    for (const auto& variable : variables) {
        std::cout << "Variable ID: " << variable.id << ", Group ID: " << variable.groupID
                  << ", Course ID: " << variable.courseId << ", Is Lecture: " << (variable.isLecture ? "Yes" : "No")
                  << ", Time Slot: " << variable.timeSlot << ", Teacher ID: " << variable.teacherID
                  << ", Is Assigned: " << (variable.isAssigned ? "Yes" : "No") << std::endl;
    }
}

void performCSP(std::vector<Variable>& variables, std::vector<Domain>& domains, const Data& data) {
    // Creating teacher_work_hours
    std::unordered_map<int, int> teacher_work_hours;

    for (const auto& teacher : data.teachers) {
        teacher_work_hours[teacher.id] = teacher.maxHours;
    }

    // Tracking lectures for each teacher in <teacher_id, set of <subject_id, time_slot>>
    std::unordered_map<int, std::unordered_set<std::pair<int, int>, pair_hash>> teacherLectures;

    bool allAssigned = false;

    while (!allAssigned) {

        // MRV (Minimum Remaining Values) heuristic to select the variable with the smallest domain
        int minDomainSize = INT_MAX;
        int selectedVarIndex = -1;

        for (size_t i = 0; i < variables.size(); ++i) {
            if (!variables[i].isAssigned && domains[i].domainSlotsLeft < minDomainSize) {
                minDomainSize = domains[i].domainSlotsLeft;
                selectedVarIndex = i;
            }
        }

        if (selectedVarIndex == -1) {
            // All variables are assigned, or there's an issue
            std::cout << "All variables are assigned." << std::endl;
            allAssigned = true;
            break;
        }

        // Assigning a domain slot to the selected variable
        Domain& selectedDomain = domains[selectedVarIndex];
        Variable& selectedVar = variables[selectedVarIndex];

        if (selectedDomain.domainSlots.empty()) {
            std::cout << "No available domain slots for the selected variable." << std::endl;
            return;
        }

        //Choosing the domain slot with the teacher having the maximum work hours left (least busy teacher)
        int maxTeacherID = -1;
        int maxHoursLeft = -1;
        for (const auto& slot : selectedDomain.domainSlots) {
            if (teacher_work_hours[slot.teacherID] > maxHoursLeft) {
                maxHoursLeft = teacher_work_hours[slot.teacherID];
                maxTeacherID = slot.teacherID;
            }
        }

        DomainSlot chosenSlot;
        for (auto it = selectedDomain.domainSlots.begin(); it != selectedDomain.domainSlots.end(); ++it) {
            if (it->teacherID == maxTeacherID) {
                chosenSlot = *it;
                // selectedDomain.domainSlots.erase(it);
                break;
            }
        }

        // Updating variable and domain information after assignment
        selectedVar.timeSlot = chosenSlot.timeSlot;
        selectedVar.teacherID = chosenSlot.teacherID;
        selectedVar.isAssigned = true;
        //selectedDomain.updateDomainSlotsLeft();

        // If a lecture, add it to teacherLectures
        if (selectedVar.isLecture) {
            teacherLectures[selectedVar.teacherID].emplace(selectedVar.courseId, chosenSlot.timeSlot);
        }

        // Decrement work hours of the assigned teacher except for the same lecture times
        if (teacherLectures.find(selectedVar.teacherID) != teacherLectures.end()) {
            const auto& lectureSet = teacherLectures[selectedVar.teacherID];
            bool isSameLectureTime = lectureSet.find({ selectedVar.courseId, chosenSlot.timeSlot }) != lectureSet.end();
            if (!isSameLectureTime) {
                teacher_work_hours[selectedVar.teacherID]--;
            }
        }

        // Cross out conflicting slots from other domains
        for (size_t i = 0; i < variables.size(); ++i) {
            const Variable& currentVar = variables[i];
            if (!currentVar.isAssigned) {
                Domain& currentDomain = domains[i];

                // Condition 1: For variables with the same group, delete slots with the same time slot
                if (currentVar.groupID == selectedVar.groupID) {
                    for (auto it = currentDomain.domainSlots.begin(); it != currentDomain.domainSlots.end();) {
                        if (it->timeSlot == chosenSlot.timeSlot) {
                            it = currentDomain.domainSlots.erase(it);
                            currentDomain.updateDomainSlotsLeft();
                        } else {
                            ++it;
                        }
                    }
                }

                // Condition 2: For unassigned variables, delete slots with the same time slot and teacher
                for (auto it = currentDomain.domainSlots.begin(); it != currentDomain.domainSlots.end();) {
                    if (!variables[i].isAssigned && it->teacherID == chosenSlot.teacherID && it->timeSlot == chosenSlot.timeSlot) {
                        if (!(currentVar.isLecture && selectedVar.isLecture && currentVar.courseId == selectedVar.courseId)) {
                            it = currentDomain.domainSlots.erase(it);
                            currentDomain.updateDomainSlotsLeft();
                        } else {
                            ++it;
                        }
                    } else {
                        ++it;
                    }
                }

                // Condition 3: If teacher's max hours is already 0, cross out all domain slots with this teacher
                if (teacher_work_hours[chosenSlot.teacherID] == 0) {
                    for (auto it = currentDomain.domainSlots.begin(); it != currentDomain.domainSlots.end();) {
                        if (it->teacherID == chosenSlot.teacherID) {
                            it = currentDomain.domainSlots.erase(it);
                            currentDomain.updateDomainSlotsLeft();
                        } else {
                            ++it;
                        }
                    }
                }

            }
        }

        //printVariables(variables);
        //std::cout<<"================================================="<<std::endl;
        std::cout<<"For Variable"<<selectedVar.id<<"  setted value <teacher: "<<selectedVar.teacherID<<"; time: "<<selectedVar.timeSlot<<">"<<std::endl;

    }
}

int main() {
    Data data; 
    std::vector<Variable> variables;

    // Generate variables from Data
    for (const auto& group : data.groups) {
        for (const auto& course : group.courses) {
            for (int lectureId = 0; lectureId < course.lectureHours; ++lectureId) {
                variables.emplace_back(Variable(variables.size(), group.id, course.id, true));
            }
            for (int labId = 0; labId < course.labHours; ++labId) {
                variables.emplace_back(Variable(variables.size(), group.id, course.id, false));
            }
        }
    }

    // Create domains
    std::vector<Domain> domains;

    for (const auto& variable : variables) {
        std::vector<DomainSlot> variableDomain;

        const auto& qualifiedTeachers = data.courses[variable.courseId].qualifiedTeachers;

        for (const auto& teacher : qualifiedTeachers) {

            for (int timeSlot = 0; timeSlot < TIME_SLOTS; ++timeSlot) {
                variableDomain.emplace_back(DomainSlot(teacher.id, timeSlot));
            }
        }
        domains.emplace_back(Domain(variable.id, variableDomain));
    }

    // CSP

    performCSP(variables, domains, data);

    printVariables(variables);
    
    return 0;
}


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

constexpr int TIME_SLOTS = 20;

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
    int Hours;
    std::vector<Teacher> qualifiedTeachers; 

    Course(int _id, const std::string& _name, int _Hours, const std::vector<Teacher>& _qualifiedTeachers)
        : id(_id), name(_name), Hours(_Hours), qualifiedTeachers(_qualifiedTeachers) {}
};

class Teacher {
public:
    int id;
    std::string name;
    int maxHours;
    std::unordered_map<int, int> assignedHours; 
    std::unordered_map<int, std::set<int>> lectureAssignments;
    std::vector<Course> courses; 

    Teacher(int _id, std::string _name, int _maxHours) 
        : id(_id), name(_name), maxHours(_maxHours) {}
};

class Data {
public:
    std::vector<Teacher> teachers;
    std::vector<Group> groups;
    std::vector<Course> courses;

  Data() {

    teachers.push_back(Teacher(0, "Teacher0", 5)); // ID, Name, Max Hours
    teachers.push_back(Teacher(1, "Teacher1", 10));
    teachers.push_back(Teacher(2, "Teacher2", 16));
    teachers.push_back(Teacher(3, "Teacher3", 10));
    teachers.push_back(Teacher(4, "Teacher4", 10));
    teachers.push_back(Teacher(5, "Teacher5", 20));


    Course course0(0, "Course0",  2, {teachers[2], teachers[1]});//id, name, lectures_hours, qualified teachers
    Course course1(1, "Course1", 2, {teachers[0], teachers[1]});
    Course course2(2, "Course2", 4, {teachers[4]});
    Course course3(3, "Course3", 1, {teachers[0]});
    Course course4(4, "Course4", 3, {teachers[4], teachers[2]});
    Course course5(5, "Course5", 2, {teachers[0], teachers[3]});
    Course course6(6, "Course6", 3, {teachers[0], teachers[5]});
    Course course7(7, "Course7", 2, {teachers[3], teachers[4]});
    Course course8(8, "Course8", 1, {teachers[0], teachers[5]});
    Course course9(9, "Course9", 4, {teachers[3], teachers[5]});

    courses.push_back(course0);
    courses.push_back(course1);
    courses.push_back(course2);
    courses.push_back(course3);
    courses.push_back(course4);
    courses.push_back(course5);
    courses.push_back(course6);
    courses.push_back(course7);
    courses.push_back(course8);
    courses.push_back(course9);


    std::vector<Course> group1Courses = {course1, course2, course0};
    Group group1(0, "Group1", group1Courses);

    std::vector<Course> group2Courses = {course1, course3, course4,course5, course6};
    Group group2(1, "Group2", group2Courses);

    std::vector<Course> group3Courses = {course1, course2, course3, course0};
    Group group3(2, "Group3", group3Courses);

    std::vector<Course> group4Courses = {course5, course6, course7, course8, course0};
    Group group4(3, "Group4", group4Courses);

    groups.push_back(group1);
    groups.push_back(group2);
    groups.push_back(group3);
    groups.push_back(group4);

    for (auto& course : courses) {
        for (auto& teacher : course.qualifiedTeachers) {
            teachers[teacher.id].courses.push_back(course);
        }
    }
}

};

void printSchedule(const std::vector<std::vector<std::pair<std::string, std::string>>>& schedule) {
    for (size_t i = 0; i < schedule.size(); ++i) {
        std::cout << "Group " << i + 1 << " Schedule:" << std::endl;
        for (size_t j = 0; j < schedule[i].size(); ++j) {
            std::cout << "Hour " << j + 1 << ": ";
            if (schedule[i][j].first.empty()) {
                std::cout << "No class assigned";
            } else {
                std::cout << "Subject: " << schedule[i][j].first << ", Teacher: " << schedule[i][j].second;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}


bool peek(const std::unordered_map<std::string, int>& course_needed, const std::unordered_map<std::string, int>& course_pool) {
    for (const auto& [subject, needed] : course_needed) {
        if (course_pool.find(subject) != course_pool.end() && needed > course_pool.at(subject)) {
            return false;
        }
    }
    return true;
}

void set_hours(
    std::unordered_map<std::string, int>& course_needed,
    std::unordered_map<std::string, int>& course_pool,
    const std::string& subject,
    const Teacher& teacher,
    int a
) {
    course_needed[subject] -= a;
    for (const auto& subj : teacher.courses) {
        course_pool[subj.name] -= a;
    }

}

bool teacherSorting(const Teacher& t1, const Teacher& t2) {
    int count1 = 0, count2 = 0;
    for (const auto& course : t1.courses) {
        if (course.Hours > 0) {
            count1++;
        }
    }
    for (const auto& course : t2.courses) {
        if (course.Hours > 0) {
            count2++;
        }
    }

    // Sort teachers based on the number of qualified subjects (descending order)
    return count1 > count2;
}

bool csp(
    std::vector<Group>& groups,
    std::unordered_map<std::string, int>& course_needed,
    std::unordered_map<std::string, int>& course_pool,
    std::vector<std::vector<std::pair<std::string, std::string>>>& schedule,
    Data& data
) {
    std::cout << "Hours needed:" << std::endl;
    for (const auto& [subject, needed] : course_needed) {
        std::cout << subject << ": " << needed << std::endl;
    }

    std::cout << "\nHours in pool:" << std::endl;
    for (const auto& [subject, supply] : course_pool) {
        std::cout << subject << ": " << supply << std::endl;
    }

    for (auto& group : groups) {
        for (size_t hour = 0; hour < schedule[group.id].size(); ++hour) {
            if (schedule[group.id][hour].first.empty()) {
                for (auto& subject : group.courses) {
                    if (subject.Hours == 0) {
                        continue;
                    }

                    std::sort(data.teachers.begin(), data.teachers.end(), teacherSorting);

                    for (auto& teacher : data.teachers) {
                        bool isQualified = false;
                        for (const auto& qualCourse : teacher.courses) {
                            if (qualCourse.id == subject.id) {
                                isQualified = true;
                                break;
                            }
                        }

                        if (!isQualified || teacher.maxHours == 0) {
                            continue;
                        }
                        std::cout<<"Assigning varuable  < "<<group.name<<", "<<teacher.name<<", "<<subject.name<<", hour:"<<hour<<" >"<<std::endl;
                        schedule[group.id][hour] = {subject.name, teacher.name};
                        subject.Hours--;
                        teacher.maxHours--;

                        set_hours(course_needed, course_pool, subject.name, teacher, 1);

                        if (peek(course_needed, course_pool)) {
                            if (csp(groups, course_needed, course_pool, schedule, data)) {
                                return true;
                            }
                        }

                        std::cout<<"Going back..."<<std::endl;
                        schedule[group.id][hour] = {"", ""};
                        subject.Hours++;
                        teacher.maxHours++;

                        set_hours(course_needed, course_pool, subject.name, teacher, -1);
                    }
                }
            }
        }
    }
    return !std::any_of(groups.begin(), groups.end(), [](const Group& g) {
        return std::any_of(g.courses.begin(), g.courses.end(), [](const Course& c) {
            return c.Hours > 0;
        });
    });
}


int main() {
    Data data;  

    std::unordered_map<std::string, int> course_needed;
    std::unordered_map<std::string, int> course_pool;

    //populate needed and pool (hours - subject)

    for (const auto& group : data.groups) {
        for (const auto& course : group.courses) {
            course_needed[course.name] += course.Hours;
        }
    }

    for (const auto& teacher : data.teachers) {
    for (const auto& course : teacher.courses) {
        course_pool[course.name] += teacher.maxHours;
        
        //std::cout << "Course Name: " << course.name << ", Teacher Max Hours: " << teacher.maxHours << std::endl;
    }
}
    std::vector<std::vector<std::pair<std::string, std::string>>> schedule(data.groups.size(), std::vector<std::pair<std::string, std::string>>(TIME_SLOTS, {"", ""}));
    /*
    std::cout << "Hours needed:" << std::endl;
    for (const auto& [course, needed] : course_needed) {
        std::cout << course << ": " << needed << std::endl;
    }

    std::cout << "\nHours in pool:" << std::endl;
    for (const auto& [course, pool] : course_pool) {
        std::cout << course << ": " << pool << std::endl;
    }*/

    if (!csp(data.groups, course_needed, course_pool, schedule, data)) {
        std::cout << "NO SOLUTION" << std::endl;
    }
    else{std::cout << "SOLUTION FOUND!" << std::endl;
    printSchedule(schedule);}


    return 0;
}

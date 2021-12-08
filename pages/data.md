# Data

## Structure of the Data

All of the data is in JSON form. Some examples for each are included below. The data actually included in the repository may have additional fields as it was also used for another project. All fields mentioned below are manditory while additional fields are simply ignored.


### Classroom

```
{
	"code":"FASS G032",
	"capacity":40
}
```

- Code: A simple string unique to the classroom.
- Capacity: An integer denoting the capacity of the classroom.

### Course

```
{
	"code":"CS 406",
	"section":"A",
	"capacity":40,
	"instructors":["John Smith", "Jane Smith"],
	"academicBlocks":[12,54]
	"firstDuration": 2,
    "secondDuration": 1
}
```

- Code: A simple string unique to the course.
- Section: Some courses may have multiple sections. This is a simple string (or often just one character) distinguishing them.
- Capacity: An integer denoting the maximum number of students that can register for the course.
- Instructors: An array of strings, each denoting the name (or any unique identifier) of an instructor for the course.
- Academic Blocks: An array of integers, each denoting a group of courses that should not collide with each other. For example the above course is in the 12th academic block and should not have classes at the same time as any other course in the 12th academic block.
- First & Second Duration: Number of class hours the course should have. For example the above course has 3 hours per week as one 2 hour, one 1 hour class time. 

### Concurrent

```
{
	"courseCode1": "CS 305",
	"courseCode2": "IF 200",
	"studentCount": 10
}
```

- Course Codes: Same codes used in the course data. Order does not matter but there should be only 1 entry for each pair of courses. Any pair missing will simply be assumed to have a student count of zero.
- Student Count: Number of students who have taken the two courses together in one term (concurrently as we call it) in past years.

## Source of the Data

All of the data used in the project belongs to Sabancı University. Course data was parsed directly from the student information system website. The rest of the data (classrooms, concurrency, academic blocks) was given to us by our instructor. Some of the data included in the repository may be altered 


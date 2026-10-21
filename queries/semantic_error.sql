-- Expected: S004 (ambiguous unqualified column)
SELECT department, name
FROM students, courses
WHERE students.department = courses.department;

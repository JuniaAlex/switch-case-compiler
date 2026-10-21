-- Three-table join with aliases and an alias in ORDER BY
SELECT s.name, c.title, e.grade
FROM students s, enrollments e, courses c
WHERE s.student_id = e.student_id
  AND e.course_id = c.course_id
  AND e.semester = 5
ORDER BY s.name ASC, c.title;

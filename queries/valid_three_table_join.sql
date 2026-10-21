-- Valid three-table join with expression, alias, and ordering
SELECT s.name, c.title, e.grade, (s.cgpa * 10) AS percentage
FROM students s, enrollments e, courses c
WHERE s.student_id = e.student_id
  AND e.course_id = c.course_id
  AND s.cgpa >= 8.0
ORDER BY percentage DESC;

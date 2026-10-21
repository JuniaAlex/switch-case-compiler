-- Basic projection and filter
SELECT name, cgpa, city
FROM students
WHERE age >= 18 AND cgpa >= 7.5
ORDER BY cgpa DESC, name ASC;

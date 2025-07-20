
for i in {1..100000}; do
   time curl -X GET http://localhost:8888/test
   echo "Sending Request: ${i}"
done

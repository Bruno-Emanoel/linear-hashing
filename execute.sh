
mkdir bin

g++ ./src/gen.cpp -o ./bin/gen -Ofast
./bin/gen <<< "50 100000 5000"
g++ ./src/main.cpp -o ./bin/hashing -O2

for p in 10 25 50 100  
do
    for alfa in 0.6 0.75 0.8 0.9
    do
        for file in tests/* 
        do
            echo "Resolvendo para os parâmetros P=$p, alfaMax=$alfa. Teste=$file"
            ./bin/hashing $p $alfa results/table.csv $file < $file 
        done
    done
done

rm -r ./bin
for i in {0..500}; do
  deno run -A scripts/index.js $1 &
done
wait


// tcp 
const encoder = new TextEncoder()
const decoder = new TextDecoder()
async function main() {
  const startTime = performance.now()
  const firstArg = Deno.args[0] ?? ""

  const conn = await Deno.connect({ hostname: "0.0.0.0", port: 8080 })

  await conn.write(encoder.encode(`/tmp/testfiles/${firstArg}.c\n`))

  const reader = conn.readable.getReader()
  while (true) {
    const { value, done } = await reader.read()
    if (done) break
    // console.log(decoder.decode(value))
  }
  const endTime = performance.now()
  console.log(`Time taken: ${endTime - startTime}ms`)
}
main()

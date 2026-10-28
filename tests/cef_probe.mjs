// Local renderer test helper. DevTools is enabled only by the integration test.
const [port, expression] = process.argv.slice(2);
const pages = await (await fetch(`http://127.0.0.1:${port}/json/list`)).json();
const page = pages.find(p => p.type === 'page');
if (!page) throw new Error('No wallpaper page');
const socket = new WebSocket(page.webSocketDebuggerUrl);
await new Promise((resolve, reject) => { socket.onopen = resolve; socket.onerror = reject; });
const answer = new Promise(resolve => { socket.onmessage = e => { const m = JSON.parse(e.data); if (m.id === 1) resolve(m); }; });
socket.send(JSON.stringify({id: 1, method: 'Runtime.evaluate', params: {expression, returnByValue: true}}));
const result = await answer;
socket.close();
if (result.error || result.result.exceptionDetails) throw new Error(JSON.stringify(result));
console.log(JSON.stringify(result.result.result.value ?? null));

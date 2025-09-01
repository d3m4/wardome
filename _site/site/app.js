async function loadCatalog() {
  const url = '../out/catalog.json';
  const res = await fetch(url);
  if (!res.ok) throw new Error('Failed to load catalog.json');
  return await res.json();
}

function el(tag, attrs={}, children=[]) {
  const e = document.createElement(tag);
  for (const [k,v] of Object.entries(attrs)) {
    if (k === 'class') e.className = v; else if (k === 'html') e.innerHTML = v; else e.setAttribute(k,v);
  }
  for (const c of children) e.appendChild(typeof c === 'string' ? document.createTextNode(c) : c);
  return e;
}

function linkTo(file, text) {
  const a = el('a', { href: `../out/${file}`, target: '_blank' }, [text || file]);
  return a;
}

function renderSummary(container, cat) {
  const s = cat.summary || {};
  const tbl = el('table');
  const rows = [
    ['Rooms', (s.rooms||0)],
    ['Zones', (s.zones||0)],
    ['Objects', (s.objects||0)],
    ['Mobs', (s.mobs||0)],
    ['Shops', (s.shops||0)],
    ['Help', (s.help_entries||0)],
  ];
  for (const [k,v] of rows) tbl.appendChild(el('tr',{},[el('th',{},[k]), el('td',{},[String(v)])]));
  container.innerHTML = '';
  container.appendChild(tbl);
}

function renderList(view, cat, listEl, detailsEl, opts={}) {
  listEl.innerHTML = '';
  detailsEl.innerHTML = '';
  const q = (opts.query||'').toLowerCase();
  let data = cat[view];
  if (!Array.isArray(data)) { listEl.textContent = 'No data'; return; }
  if (q) {
    data = data.filter(it => {
      const id = String(it.id||'');
      const name = String(it.name||it.short_desc||'');
      const keywords = Array.isArray(it.keywords)? it.keywords.join(' '): '';
      return id.includes(q) || name.toLowerCase().includes(q) || keywords.toLowerCase().includes(q);
    });
  }
  const tbl = el('table');
  const header = el('tr');
  const cols = view === 'help' ? ['keywords','file'] : ['id','name','file'];
  for (const c of cols) header.appendChild(el('th',{},[c]));
  tbl.appendChild(header);
  for (const item of data) {
    const tr = el('tr');
    if (view === 'help') {
      tr.appendChild(el('td',{},[Array.isArray(item.keywords)? item.keywords.join(', '): '']));
      tr.appendChild(el('td',{},[linkTo(item.file, 'open')]))
    } else {
      tr.appendChild(el('td',{},[String(item.id || '')]));
      tr.appendChild(el('td',{},[String(item.name || item.short_desc || '')]));
      tr.appendChild(el('td',{},[linkTo(item.file, 'open')]));
    }
    tr.addEventListener('click', async () => {
      const res = await fetch(`../out/${item.file}`);
      const obj = await res.json();
      detailsEl.innerHTML = '';
      detailsEl.appendChild(el('h3',{},[`${view.slice(0,-1)} ${item.id || ''}`]));
      // Enrich room sector if possible
      if (view === 'rooms') {
        const sectorIdx = obj.sector;
        const sectorName = (window._constants&&window._constants.sector_types&&Number.isInteger(sectorIdx))? window._constants.sector_types[sectorIdx] : undefined;
        if (sectorName) detailsEl.appendChild(el('div',{},[el('b',{},['Sector: ']), sectorName]));
      }
      detailsEl.appendChild(el('pre',{},[JSON.stringify(obj, null, 2)]));
    });
    tbl.appendChild(tr);
  }
  listEl.appendChild(tbl);
}

function renderTables(cat, listEl, detailsEl) {
  listEl.innerHTML = '';
  detailsEl.innerHTML = '';
  const t = cat.tables || {};
  // Spells
  if (t.spells && Array.isArray(t.spells.spells)) {
    const h = el('h3',{},['Spells']);
    const ul = el('ul');
    for (const s of t.spells.spells.slice(0,300)) ul.appendChild(el('li',{},[`#${s.id} ${s.name}`]));
    listEl.appendChild(h); listEl.appendChild(ul);
  }
  // Skills
  if (t.skills && Array.isArray(t.skills.skills)) {
    const h = el('h3',{},['Skills']);
    const ul = el('ul');
    for (const s of t.skills.skills) ul.appendChild(el('li',{},[`#${s.id} ${s.name}`]));
    listEl.appendChild(h); listEl.appendChild(ul);
  }
  // Commands
  if (t.commands && Array.isArray(t.commands.commands)) {
    const h = el('h3',{},['Commands']);
    const tbl = el('table');
    const head = el('tr');
    for (const c of ['command','alias','position','function','level','subcmd']) head.appendChild(el('th',{},[c]));
    tbl.appendChild(head);
    for (const c of t.commands.commands) {
      const tr = el('tr');
      tr.appendChild(el('td',{},[c.command||'']));
      tr.appendChild(el('td',{},[c.alias||'']));
      tr.appendChild(el('td',{},[c.minimum_position||'']));
      tr.appendChild(el('td',{},[c.function||'']));
      tr.appendChild(el('td',{},[c.minimum_level||'']));
      tr.appendChild(el('td',{},[c.subcmd||'']));
      tbl.appendChild(tr);
    }
    listEl.appendChild(h); listEl.appendChild(tbl);
  }
  // Classes
  if (t.classes && Array.isArray(t.classes.values)) {
    const h = el('h3',{},['Classes']);
    const ul = el('ul');
    for (const c of t.classes.values) ul.appendChild(el('li',{},[c]));
    listEl.appendChild(h); listEl.appendChild(ul);
  }
  // Constants
  if (t.constants) {
    const blocks = [
      ['sector_types','Sector Types'],
      ['room_bits','Room Flags'],
      ['exit_bits','Exit Flags'],
      ['zone_bits','Zone Flags']
    ];
    for (const [key, title] of blocks) {
      const arr = t.constants[key];
      if (!arr) continue;
      const h = el('h3',{},[title]);
      const ul = el('ul');
      arr.forEach((name, idx) => {
        if (name === '\\n') return; // trailing sentinel
        ul.appendChild(el('li',{},[`#${idx}: ${name}`]));
      });
      listEl.appendChild(h); listEl.appendChild(ul);
    }
  }
}

(async function() {
  const cat = await loadCatalog();
  const summaryEl = document.getElementById('summary');
  const listEl = document.getElementById('list');
  const detailsEl = document.getElementById('details');
  // cache constants for enrichment
  window._constants = (cat.tables && cat.tables.constants) || {};
  renderSummary(summaryEl, cat);
  const setView = (v, opts={}) => {
    window._view = v;
    window._opts = opts;
    if (v === 'tables') renderTables(cat, listEl, detailsEl);
    else if (v === 'zoneview') renderZoneView(cat, listEl, detailsEl, opts);
    else renderList(v, cat, listEl, detailsEl, opts);
  };
  const search = document.getElementById('search');
  document.querySelectorAll('nav button').forEach(b => b.addEventListener('click', () => {
    // clear search when changing categories
    if (search) search.value = '';
    setView(b.dataset.view, { query: '' });
  }));
  // live search
  search.addEventListener('input', () => {
    const v = window._view || 'zones';
    setView(v, { query: search.value });
  });
  setView('zones');
})();

function renderZoneView(cat, listEl, detailsEl, opts={}) {
  listEl.innerHTML = '';
  detailsEl.innerHTML = '';
  const q = (opts.query||'').toLowerCase();
  // zone list on left, click to list rooms of zone
  const zones = (cat.zones||[]).filter(z => {
    const h = (z.name||'').toLowerCase();
    return !q || h.includes(q) || String(z.id||'').includes(q);
  });
  const tbl = el('table');
  const head = el('tr');
  for (const c of ['id','name','file']) head.appendChild(el('th',{},[c]));
  tbl.appendChild(head);
  for (const z of zones) {
    const tr = el('tr');
    tr.appendChild(el('td',{},[String(z.id)]));
    tr.appendChild(el('td',{},[z.name||'']));
    tr.appendChild(el('td',{},[linkTo(z.file, 'open')]));
    tr.addEventListener('click', () => showZoneDetails(z, cat, detailsEl));
    tbl.appendChild(tr);
  }
  listEl.appendChild(tbl);
}

async function showZoneDetails(zone, cat, container) {
  container.innerHTML = '';
  container.appendChild(el('h3',{},[`Zone ${zone.id}: ${zone.name||''}`]));
  // load zone JSON to show header and commands
  const zres = await fetch(`../out/${zone.file}`);
  const zjson = await zres.json();
  container.appendChild(el('pre',{},[JSON.stringify({header: zjson.header, commands: zjson.commands.slice(0,100)}, null, 2)]));
  // list rooms in this zone (fallback to header range if zone_id missing)
  let rooms = (cat.rooms||[]).filter(r => r.zone_id === zone.id);
  if (!rooms.length && zjson && zjson.header) {
    const minr = Number.isInteger(zjson.header.min_room) ? zjson.header.min_room : null;
    const top = Number.isInteger(zjson.header.top_room) ? zjson.header.top_room : null;
    if (top != null) {
      rooms = (cat.rooms||[]).filter(r => {
        const id = r.id|0; return (minr==null || id >= minr) && id <= top;
      });
    }
  }
  await renderZoneGraph(zone, rooms, container);
  const tbl = el('table');
  const head = el('tr');
  for (const c of ['id','name','open']) head.appendChild(el('th',{},[c]));
  tbl.appendChild(head);
  for (const r of rooms) {
    const tr = el('tr');
    tr.appendChild(el('td',{},[String(r.id)]));
    tr.appendChild(el('td',{},[r.name||'']));
    const open = linkTo(r.file, 'open');
    tr.appendChild(el('td',{},[open]));
    tr.addEventListener('click', async () => {
      const res = await fetch(`../out/${r.file}`);
      const obj = await res.json();
      const sectorIdx = obj.sector;
      const sectorName = (window._constants&&window._constants.sector_types&&Number.isInteger(sectorIdx))? window._constants.sector_types[sectorIdx] : undefined;
      container.appendChild(el('h4',{},[`Room ${r.id}`]));
      if (sectorName) container.appendChild(el('div',{},[el('b',{},['Sector: ']), sectorName]));
      container.appendChild(el('pre',{},[JSON.stringify(obj, null, 2)]));
    });
    tbl.appendChild(tr);
  }
  container.appendChild(el('h3',{},[`Rooms in Zone ${zone.id}`]));
  container.appendChild(tbl);
}

async function renderZoneGraph(zone, rooms, container) {
  // Fetch room details to get exits
  const details = [];
  for (const r of rooms) {
    try {
      const res = await fetch(`../out/${r.file}`);
      const obj = await res.json();
      details.push(obj);
    } catch {}
  }
  if (!details.length) {
    container.appendChild(el('div',{},['No rooms found for this zone.']));
    return;
  }
  // Build adjacency within zone (dirs 0..3 only for 2D)
  const inZone = new Set(details.map(d => d.id));
  const adj = new Map();
  for (const d of details) {
    const ex = d.exits||[];
    const list = [];
    for (const e of ex) {
      if (e.to != null && inZone.has(e.to) && [0,1,2,3].includes(e.dir)) {
        list.push({dir:e.dir, to:e.to});
      }
    }
    adj.set(d.id, list);
  }
  // BFS layout with grid coordinates
  const pos = new Map();
  const visited = new Set();
  const queue = [];
  if (details.length === 0) return;
  const seed = details[0].id;
  queue.push({id: seed, x:0, y:0});
  visited.add(seed);
  pos.set(seed, {x:0,y:0});
  const delta = {0:[0,-1],1:[1,0],2:[0,1],3:[-1,0]};
  while (queue.length) {
    const cur = queue.shift();
    const neigh = adj.get(cur.id)||[];
    for (const {dir,to} of neigh) {
      if (visited.has(to)) continue;
      const [dx,dy] = delta[dir];
      const nx = cur.x + dx;
      const ny = cur.y + dy;
      pos.set(to, {x:nx,y:ny});
      visited.add(to);
      queue.push({id:to,x:nx,y:ny});
    }
  }
  // Place unvisited rooms in a separate row
  let ux = 0; const uy = (Math.max(0,...[...pos.values()].map(p=>p.y))+2);
  for (const d of details) {
    if (!pos.has(d.id)) { pos.set(d.id, {x:ux++, y:uy}); }
  }
  // If no adjacency, place nodes in a simple grid
  const hasEdges = [...adj.values()].some(l => (l||[]).length);
  if (!hasEdges) {
    let gx=0, gy=0, perRow=10;
    for (const d of details) { pos.set(d.id, {x: gx, y: gy}); if (++gx>=perRow){gx=0; gy++;} }
  }
  // Normalize coords and build SVG
  const coords = [...pos.values()];
  const xs = coords.map(p=>p.x), ys = coords.map(p=>p.y);
  const minX = Math.min(...xs), maxX = Math.max(...xs);
  const minY = Math.min(...ys), maxY = Math.max(...ys);
  const tile = 60; const pad = 40;
  const width = (maxX-minX+1)*tile + pad*2;
  const height = (maxY-minY+1)*tile + pad*2;
  const svg = el('svg', {class:'map', viewBox:`0 0 ${width} ${height}`});
  const idxById = new Map(details.map((d,i)=>[d.id,i]));

  // Draw edges
  for (const d of details) {
    const from = pos.get(d.id); if (!from) continue;
    for (const e of (adj.get(d.id)||[])) {
      const to = pos.get(e.to); if (!to) continue;
      const x1 = (from.x-minX)*tile+pad, y1=(from.y-minY)*tile+pad;
      const x2 = (to.x-minX)*tile+pad, y2=(to.y-minY)*tile+pad;
      svg.appendChild(el('line', {class:'edge', x1, y1, x2, y2}));
    }
  }
  // Draw nodes
  const q = (document.getElementById('search')?.value||'').toLowerCase();
  for (const d of details) {
    const p = pos.get(d.id); if (!p) continue;
    const x = (p.x-minX)*tile+pad, y=(p.y-minY)*tile+pad;
    const g = el('g', {class:'node'});
    const label = (d.name||'').toString();
    const match = q && (String(d.id).includes(q) || label.toLowerCase().includes(q));
    if (match) g.classList.add('highlight');
    const circle = el('circle', {cx:x, cy:y, r:16});
    circle.addEventListener('click', async () => {
      const res = await fetch(`../out/rooms/${d.id}.json`);
      const obj = await res.json();
      const detailsEl = document.getElementById('details');
      detailsEl.appendChild(el('h4',{},[`Room ${d.id}`]));
      detailsEl.appendChild(el('pre',{},[JSON.stringify(obj, null, 2)]));
    });
    const text = el('text', {x:x+20, y:y+4}, [`${d.id} ${label.substring(0,24)}`]);
    g.appendChild(circle); g.appendChild(text);
    svg.appendChild(g);
  }
  container.appendChild(el('h3',{},['Zone Graph (N/E/S/W)']));
  container.appendChild(svg);
}

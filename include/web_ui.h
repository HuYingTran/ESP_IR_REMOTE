#pragma once
#include <pgmspace.h>

// Giao diện web: dạng app, cố định vừa 1 màn hình điện thoại (không cuộn dọc)
static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="vi"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover">
<meta name="theme-color" content="#0f172a">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="IR Remote">
<title>IR Remote</title>
<style>
:root{--bg:#0f172a;--card:#1e293b;--card2:#0b1222;--line:#334155;--txt:#f1f5f9;--mut:#94a3b8;--pri:#3b82f6;--ok:#22c55e;--bad:#ef4444;--warn:#f59e0b;--shadow:0 8px 30px #0006;--gap:10px}
@media (prefers-color-scheme:light){:root{--bg:#f1f5f9;--card:#fff;--card2:#f8fafc;--line:#e2e8f0;--txt:#0f172a;--mut:#64748b;--shadow:0 8px 30px #0f172a22}}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;height:100%;overflow:hidden;overscroll-behavior:none;background:var(--bg);color:var(--txt);font:15px/1.35 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
/* Khung app cố định đúng chiều cao màn hình */
body{position:fixed;inset:0;display:flex;flex-direction:column;height:100vh;height:100dvh}
button{font:inherit;color:inherit;touch-action:manipulation;-webkit-user-select:none;user-select:none;-webkit-touch-callout:none;cursor:pointer}
input,select{font-size:16px}
header{flex:none;padding:calc(8px + env(safe-area-inset-top)) 16px 8px;display:flex;align-items:center;justify-content:space-between}
header h1{font-size:18px;margin:0}
.pill{font-size:12px;padding:4px 10px;border-radius:99px;background:var(--card);border:1px solid var(--line);color:var(--mut);display:flex;gap:6px;align-items:center;max-width:55%;overflow:hidden;white-space:nowrap}
.dot{flex:none;width:8px;height:8px;border-radius:50%;background:var(--mut)}.dot.ok{background:var(--ok)}.dot.warn{background:var(--warn)}
main{flex:1;min-height:0;overflow:hidden;width:100%;max-width:640px;margin:0 auto;padding:0 14px 10px;position:relative}
.tab{display:none;height:100%;flex-direction:column;gap:var(--gap)}.tab.on{display:flex;animation:fade .2s}
@keyframes fade{from{opacity:0}}
.fill{flex:1;min-height:0}
nav{flex:none;display:flex;background:var(--card);border-top:1px solid var(--line);padding:4px 6px calc(4px + env(safe-area-inset-bottom))}
body.kb nav{display:none} /* ẩn thanh tab khi bàn phím mở */
nav button{flex:1;border:0;background:none;padding:5px 0;border-radius:12px;display:flex;flex-direction:column;align-items:center;gap:1px;color:var(--mut);font-size:11px}
nav button i{font-style:normal;font-size:21px;line-height:1.15;filter:grayscale(1);opacity:.7}
nav button.on{color:var(--pri);font-weight:600}nav button.on i{filter:none;opacity:1}
.card{background:var(--card);border:1px solid var(--line);border-radius:18px;padding:14px}
.card h2{font-size:16px;margin:0 0 10px;display:flex;justify-content:space-between;align-items:center;gap:8px}
.muted{color:var(--mut);font-size:13px}.center{text-align:center}
.b{display:flex;align-items:center;justify-content:center;gap:6px;width:100%;min-height:48px;padding:10px 14px;border:0;border-radius:14px;background:var(--pri);color:#fff;font-size:16px;font-weight:600}
.b:active{filter:brightness(.88);transform:scale(.98)}
.b.sec{background:var(--card2);color:var(--txt);border:1px solid var(--line)}.b.bad{background:var(--bad)}.b.ok{background:var(--ok)}
.b.sm{min-height:36px;width:auto;padding:6px 12px;font-size:13px;border-radius:10px}
.stack{display:flex;flex-direction:column;gap:var(--gap)}.row{display:flex;gap:var(--gap)}.row>*{flex:1}
label{display:block;font-size:12px;color:var(--mut);margin:8px 0 4px}
input,select{width:100%;padding:12px 14px;border-radius:12px;border:1px solid var(--line);background:var(--card2);color:var(--txt);outline:none}
input:focus,select:focus{border-color:var(--pri)}
.tag{display:inline-block;padding:3px 10px;border-radius:99px;background:var(--card2);border:1px solid var(--line);font-size:13px}
/* Thanh chọn thiết bị (cuộn ngang) */
.hbar{flex:none;display:flex;gap:8px;overflow-x:auto;scrollbar-width:none;padding:2px 0}
.hbar::-webkit-scrollbar{display:none}
.hbar button{flex:none;padding:8px 14px;border-radius:99px;border:1px solid var(--line);background:var(--card);color:var(--mut);font-size:14px;white-space:nowrap}
.hbar button.on{background:var(--pri);border-color:var(--pri);color:#fff;font-weight:600}
/* Lưới nút remote: chia trang theo chiều ngang khi không đủ chỗ */
.pages{display:flex;overflow-x:auto;scroll-snap-type:x mandatory;scrollbar-width:none;height:100%}
.pages::-webkit-scrollbar{display:none}
.page{flex:0 0 100%;scroll-snap-align:start;display:grid;gap:var(--gap);height:100%}
.key{min-height:0;padding:6px;border:1px solid var(--line);border-radius:16px;background:var(--card);font-size:15px;font-weight:500;word-break:break-word;overflow:hidden;transition:transform .08s,background .08s}
.key:active,.key.hit{transform:scale(.94);background:var(--pri);color:#fff;border-color:var(--pri)}
.key.add{border-style:dashed;color:var(--mut);font-size:26px;background:none}
.dots{flex:none;display:flex;justify-content:center;gap:6px;height:8px}
.dots span{width:7px;height:7px;border-radius:50%;background:var(--line)}.dots span.on{background:var(--pri)}
.hint{flex:none;font-size:12px;color:var(--mut);text-align:center}
.empty{display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;gap:8px}
.big{font-size:56px;line-height:1}
/* Điều hòa */
.acTop{flex:none;display:flex;gap:var(--gap);align-items:center}
.acTop .brand{flex:1;min-width:0;display:flex;align-items:center;gap:8px;padding:0 14px;height:52px;border-radius:14px;border:1px solid var(--line);background:var(--card);text-align:left}
.acTop .brand span{flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.powbtn{flex:none;width:52px;height:52px;border-radius:50%;border:0;font-size:24px;background:var(--line);color:#fff}
.powbtn.on{background:var(--ok);box-shadow:0 0 18px #22c55e88}
.tempbox{flex:1;min-height:110px;display:flex;align-items:center;justify-content:space-between;padding:0 14px;border-radius:22px;border:1px solid var(--line);background:var(--card);transition:background .3s}
.tempbox.m1{background:linear-gradient(160deg,#1d4ed855,var(--card) 70%)}
.tempbox.m2{background:linear-gradient(160deg,#ea580c55,var(--card) 70%)}
.tempbox.m3{background:linear-gradient(160deg,#0891b255,var(--card) 70%)}
.tempbox.m4,.tempbox.m0{background:linear-gradient(160deg,#16a34a44,var(--card) 70%)}
.tempbox.off{background:var(--card);opacity:.6}
.tv{text-align:center}.tv b{display:block;font-size:clamp(48px,13vh,96px);font-weight:700;line-height:1}.tv span{color:var(--mut);font-size:14px}
.circle{flex:none;width:clamp(56px,9vh,72px);height:clamp(56px,9vh,72px);border-radius:50%;font-size:30px;border:1px solid var(--line);background:var(--card2)}
.circle:active{background:var(--pri);color:#fff}
.grp{flex:none}.grp label{margin:0 0 4px 2px}
.chips{display:grid;grid-auto-flow:column;grid-auto-columns:1fr;gap:6px}
.chips button{height:clamp(42px,6.5vh,54px);padding:2px;border-radius:12px;border:1px solid var(--line);background:var(--card);color:var(--mut);font-size:11px;display:flex;flex-direction:column;align-items:center;justify-content:center;line-height:1.15}
.chips button i{font-style:normal;font-size:17px}
.chips button.on{background:var(--pri);color:#fff;border-color:var(--pri)}
/* Học lệnh */
.lview{display:none;height:100%;flex-direction:column;gap:var(--gap)}.lview.on{display:flex}
.lcenter{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center;gap:12px;padding:0 10px}
.learnbtn{width:clamp(130px,26vh,180px);height:clamp(130px,26vh,180px);border-radius:50%;display:flex;flex-direction:column;align-items:center;justify-content:center;border:0;background:var(--pri);color:#fff;font-size:15px;font-weight:600;box-shadow:0 10px 30px #3b82f655}
.learnbtn b{font-size:46px;line-height:1}
.learnbtn.wait{animation:pulse 1.4s infinite;background:var(--warn);box-shadow:none}
@keyframes pulse{0%{box-shadow:0 0 0 0 #f59e0b88}100%{box-shadow:0 0 0 36px #f59e0b00}}
.capHead{display:flex;align-items:center;gap:8px}
.devchips{display:flex;gap:6px;overflow-x:auto;scrollbar-width:none;margin-top:6px}.devchips::-webkit-scrollbar{display:none}
.devchips button{flex:none;padding:6px 12px;border-radius:99px;border:1px solid var(--line);background:var(--card2);font-size:13px}
/* Cài đặt */
.kv{display:grid;grid-template-columns:auto 1fr;gap:6px 14px;font-size:14px}.kv span{color:var(--mut)}.kv b{font-weight:500;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.list{max-height:55vh;overflow-y:auto}
.list button{width:100%;display:flex;justify-content:space-between;align-items:center;padding:14px 4px;border:0;border-bottom:1px solid var(--line);background:none;text-align:left}
/* Bottom sheet */
#mask{position:fixed;inset:0;background:#0008;z-index:20;opacity:0;pointer-events:none;transition:.2s}
#mask.on{opacity:1;pointer-events:auto}
#sheet{position:fixed;left:0;right:0;bottom:0;z-index:21;max-width:640px;margin:0 auto;background:var(--card);border-radius:22px 22px 0 0;padding:10px 18px calc(18px + env(safe-area-inset-bottom));transform:translateY(105%);transition:transform .25s ease;box-shadow:var(--shadow);max-height:88dvh;overflow:auto}
#sheet.on{transform:none}
#sheet .grab{width:40px;height:5px;border-radius:9px;background:var(--line);margin:0 auto 12px}
#sheet h3{margin:0 0 4px;font-size:18px}
pre{white-space:pre-wrap;font-size:11px;background:var(--card2);padding:10px;border-radius:10px;color:var(--mut);max-height:40vh;overflow:auto}
/* Màn hình thấp (iPhone SE, popup captive portal...) */
@media (max-height:640px){:root{--gap:7px}header{padding-top:calc(6px + env(safe-area-inset-top));padding-bottom:6px}
 .grp label{font-size:11px;margin-bottom:2px}.chips button{height:40px;font-size:10px}.chips button i{font-size:15px}
 .tempbox{min-height:84px}.acTop .brand,.powbtn{height:46px}.powbtn{width:46px}nav button i{font-size:19px}}
@media (max-height:520px){.grp label{display:none}}
#toast{position:fixed;left:50%;bottom:calc(76px + env(safe-area-inset-bottom));transform:translate(-50%,20px);z-index:30;background:#111827;color:#fff;padding:10px 18px;border-radius:99px;opacity:0;transition:.25s;pointer-events:none;max-width:90%;font-size:14px;box-shadow:var(--shadow);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
#toast.on{opacity:1;transform:translate(-50%,0)}
</style></head><body>
<header><h1>📡 IR Remote</h1><div class="pill" id="net"><span class="dot"></span><span>...</span></div></header>
<main>

<!-- REMOTE -->
<section id="remote" class="tab on">
<div class="hbar" id="devTabs"></div>
<div class="fill" id="keyArea"></div>
<div class="dots" id="dots"></div>
<div class="hint" id="rHint">Nhấn giữ một nút để sửa / xóa</div>
</section>

<!-- ĐIỀU HÒA -->
<section id="ac" class="tab">
<div class="acTop">
<button class="brand" onclick="brandSheet()">❄️<span id="acBrand">...</span>✏️</button>
<button class="powbtn" id="acPow" onclick="acSet({power:ac.power?0:1})">⏻</button>
</div>
<div class="tempbox" id="tempBox">
<button class="circle" onclick="acTemp(-1)">−</button>
<div class="tv"><b id="acT">26°</b><span id="acSub"></span></div>
<button class="circle" onclick="acTemp(1)">+</button>
</div>
<div class="grp"><label>Chế độ</label><div class="chips" id="acMode"></div></div>
<div class="grp"><label>Quạt</label><div class="chips" id="acFan"></div></div>
<div class="grp"><label>Đảo gió</label><div class="chips" id="acSwing"></div></div>
<div class="grp"><label>Tính năng</label><div class="chips" id="acExtra"></div></div>
</section>

<!-- HỌC LỆNH -->
<section id="learn" class="tab">
<div class="lview on" id="lIdle"><div class="lcenter">
<button class="learnbtn" onclick="startLearn('btn')"><b>＋</b>Bắt đầu học</button>
<p class="muted">Hướng remote vào mắt thu IR (cách 5–15cm), bấm <b>Bắt đầu học</b> rồi bấm nút cần học trên remote.</p></div></div>

<div class="lview" id="lWait"><div class="lcenter">
<button class="learnbtn wait" onclick="cancelLearn()"><b id="lRemain">30</b>giây</button>
<p id="lWaitMsg"></p></div>
<button class="b sec" onclick="cancelLearn()">Hủy</button></div>

<div class="lview" id="lCap"><div class="card fill" style="display:flex;flex-direction:column">
<div class="capHead"><b style="flex:1">✅ Đã nhận tín hiệu</b><button class="tag" id="lProto" onclick="descSheet()"></button></div>
<p class="muted" id="lAcHint" style="display:none;margin:8px 0 0">💡 Remote điều hòa được hỗ trợ — tab <b>Điều hòa</b> chỉnh nhiệt độ tự do.</p>
<label>Thiết bị</label><input id="lDev" placeholder="VD: TV Phòng khách" autocomplete="off" enterkeyhint="next">
<div class="devchips" id="devChips"></div>
<label>Tên nút</label><input id="lName" placeholder="VD: Nguồn, Vol+, Kênh 1" autocomplete="off" enterkeyhint="done">
<div style="flex:1"></div>
<button class="b sec" style="margin-top:10px" onclick="api('learn/test',{}).then(()=>toast('📤 Đã phát thử'))">📤 Phát thử</button></div>
<button class="b ok" onclick="saveLearn(true)">💾 Lưu & học nút tiếp</button>
<div class="row"><button class="b sec" onclick="saveLearn(false)">Lưu</button><button class="b sec" onclick="startLearn('btn')">🔄 Học lại</button></div></div>

<div class="lview" id="lTo"><div class="lcenter"><div class="big">⏱</div><h2 style="margin:0">Hết thời gian</h2>
<p class="muted">Không nhận được tín hiệu. Kiểm tra mắt thu, pin remote và đưa remote lại gần hơn.</p></div>
<button class="b" onclick="startLearn(lMode)">Thử lại</button><button class="b sec" onclick="learnView('idle')">Đóng</button></div>
</section>

<!-- CÀI ĐẶT -->
<section id="set" class="tab">
<div class="card"><h2>WiFi nhà <span class="tag" id="staTag">Chưa cài</span></h2>
<div class="kv" id="staBox"></div>
<p class="muted" id="staHelp" style="margin:0">Không bắt buộc. Kết nối WiFi nhà để điều khiển khi điện thoại dùng WiFi nhà, qua <b>irremote.local</b>.</p>
<div class="row" style="margin-top:12px"><button class="b" onclick="scanSheet()">📶 Tìm WiFi</button><button class="b sec" id="forgetBtn" style="display:none" onclick="forget()">Quên WiFi</button></div></div>
<div class="card"><h2>Thông tin</h2><div class="kv" id="info"></div></div>
</section>
</main>

<nav>
<button data-t="remote" class="on"><i>🎮</i>Remote</button>
<button data-t="ac"><i>❄️</i>Điều hòa</button>
<button data-t="learn"><i>➕</i>Học lệnh</button>
<button data-t="set"><i>⚙️</i>Cài đặt</button>
</nav>
<div id="mask" onclick="closeSheet()"></div><div id="sheet"></div><div id="toast"></div>

<script>
const $=id=>document.getElementById(id);
const esc=s=>String(s??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const buzz=ms=>{try{navigator.vibrate&&navigator.vibrate(ms)}catch(e){}};
const store={get:k=>{try{return localStorage.getItem(k)}catch(e){return null}},set:(k,v)=>{try{localStorage.setItem(k,v)}catch(e){}}};
let buttons=[],ac={},acProtos=[],lMode='btn',poll=null,acTimer=null,lastDev='',curDev=store.get('dev')||'',lastDesc='';

function toast(m){const t=$('toast');t.textContent=m;t.classList.add('on');clearTimeout(t._h);t._h=setTimeout(()=>t.classList.remove('on'),1800)}
async function api(p,body){const o=body?{method:'POST',body:new URLSearchParams(body)}:{};let r,j={};
 try{r=await fetch('/api/'+p,o);j=await r.json()}catch(e){toast('❌ Mất kết nối thiết bị');throw e}
 if(!r.ok||j.ok===false){toast('❌ '+(j.error||r.status));throw j}return j}

// ---- Điều hướng ----
document.querySelectorAll('nav button').forEach(b=>b.onclick=()=>show(b.dataset.t));
function show(t){document.querySelectorAll('nav button').forEach(x=>x.classList.toggle('on',x.dataset.t==t));
 document.querySelectorAll('.tab').forEach(x=>x.classList.toggle('on',x.id==t));
 if(t=='remote')loadButtons();if(t=='ac')loadAc();if(t=='set')loadInfo();if(t=='learn')fillDevs()}
// Ẩn thanh tab khi bàn phím mở để form không bị che
document.addEventListener('focusin',e=>{if(e.target.matches('input'))document.body.classList.add('kb')});
document.addEventListener('focusout',()=>setTimeout(()=>{if(!document.activeElement.matches('input'))document.body.classList.remove('kb')},50));

// ---- Bottom sheet (thay prompt/confirm, chạy được trong popup captive portal) ----
function openSheet(html){$('sheet').innerHTML='<div class="grab"></div>'+html;$('sheet').classList.add('on');$('mask').classList.add('on')}
function closeSheet(){$('sheet').classList.remove('on');$('mask').classList.remove('on');document.activeElement&&document.activeElement.blur()}

// ---- Remote ----
const devNames=()=>[...new Set(buttons.map(b=>b.d))].sort();
async function loadButtons(){buttons=(await api('buttons')).sort((a,b)=>a.id-b.id);renderRemote()}
function renderRemote(){const ds=devNames();const area=$('keyArea');
 if(!ds.length){$('devTabs').innerHTML='';$('dots').innerHTML='';$('rHint').style.display='none';
  area.innerHTML=`<div class="empty" style="height:100%"><div class="big">📭</div><h2 style="margin:0">Chưa có nút nào</h2><p class="muted">Học lệnh từ remote TV, quạt... để điều khiển bằng điện thoại.</p><button class="b" style="width:auto" onclick="show('learn')">➕ Học lệnh đầu tiên</button></div>`;return}
 $('rHint').style.display='';
 if(!ds.includes(curDev))curDev=ds[0];
 $('devTabs').innerHTML=ds.map(d=>`<button class="${d==curDev?'on':''}">${esc(d)}</button>`).join('');
 $('devTabs').querySelectorAll('button').forEach((b,i)=>b.onclick=()=>{curDev=ds[i];store.set('dev',curDev);renderRemote()});
 // Tính số hàng/cột vừa khít vùng hiển thị
 const W=area.clientWidth,H=area.clientHeight,gap=10,cols=W>=480?4:3;
 const rows=Math.max(1,Math.floor((H+gap)/(64+gap))),per=rows*cols;
 const items=buttons.filter(b=>b.d==curDev).map(b=>`<button class="key" data-id="${b.id}">${esc(b.n)}</button>`);
 items.push('<button class="key add">＋</button>');
 const pages=[];for(let i=0;i<items.length;i+=per)pages.push(items.slice(i,i+per));
 area.innerHTML=`<div class="pages">${pages.map(p=>`<div class="page" style="grid-template-columns:repeat(${cols},1fr);grid-template-rows:repeat(${rows},1fr)">${p.join('')}</div>`).join('')}</div>`;
 $('dots').innerHTML=pages.length>1?pages.map((_,i)=>`<span class="${i?'':'on'}"></span>`).join(''):'';
 const pg=area.firstChild;pg.onscroll=()=>{const i=Math.round(pg.scrollLeft/pg.clientWidth);$('dots').querySelectorAll('span').forEach((s,k)=>s.classList.toggle('on',k==i))};
 area.querySelectorAll('.key[data-id]').forEach(bindKey);
 area.querySelector('.key.add').onclick=()=>{lastDev=curDev;show('learn');startLearn('btn')}}
let rT;addEventListener('resize',()=>{clearTimeout(rT);rT=setTimeout(()=>$('remote').classList.contains('on')&&renderRemote(),150)});
function bindKey(el){let t=null,long=false;const id=+el.dataset.id;
 const start=()=>{long=false;t=setTimeout(()=>{long=true;buzz(40);editSheet(id)},550)};
 const stop=()=>clearTimeout(t);
 el.addEventListener('pointerdown',start);['pointerup','pointerleave','pointercancel'].forEach(e=>el.addEventListener(e,stop));
 el.addEventListener('contextmenu',e=>e.preventDefault());
 el.addEventListener('click',()=>{if(!long)press(id,el)})}
async function press(id,el){buzz(25);if(el){el.classList.add('hit');setTimeout(()=>el.classList.remove('hit'),150)}
 const b=buttons.find(x=>x.id==id);await api('send',{id});toast('📤 '+b.n)}
function editSheet(id){const b=buttons.find(x=>x.id==id);openSheet(`<h3>Sửa nút</h3><p class="muted">Giao thức: ${esc(b.p)}</p>
 <label>Tên nút</label><input id="eN" value="${esc(b.n)}"><label>Thiết bị</label><input id="eD" value="${esc(b.d)}">
 <div class="stack" style="margin-top:16px"><button class="b" onclick="doRename(${id})">💾 Lưu</button>
 <div class="row"><button class="b sec" onclick="press(${id})">📤 Phát thử</button><button class="b bad" onclick="askDel(${id})">🗑 Xóa</button></div>
 <button class="b sec" onclick="closeSheet()">Đóng</button></div>`)}
async function doRename(id){const d=$('eD').value.trim();await api('rename',{id,n:$('eN').value,d});if(d)curDev=d;closeSheet();toast('Đã lưu');loadButtons()}
function askDel(id){const b=buttons.find(x=>x.id==id);openSheet(`<h3>Xóa "${esc(b.n)}"?</h3><p class="muted">Lệnh đã học sẽ bị xóa vĩnh viễn.</p>
 <div class="stack" style="margin-top:12px"><button class="b bad" onclick="doDel(${id})">Xóa</button><button class="b sec" onclick="closeSheet()">Hủy</button></div>`)}
async function doDel(id){await api('delete',{id});closeSheet();toast('🗑 Đã xóa');loadButtons()}

// ---- Học lệnh ----
function fillDevs(){$('devChips').innerHTML=devNames().map(d=>`<button type="button">${esc(d)}</button>`).join('');
 $('devChips').querySelectorAll('button').forEach(b=>b.onclick=()=>{$('lDev').value=b.textContent;$('lName').focus()})}
async function startLearn(m){lMode=m;await api('learn/start',{mode:m});if(m=='ac')show('learn');
 $('lWaitMsg').innerHTML=m=='ac'?'<b>Bấm 1 nút bất kỳ trên remote điều hòa...</b>':'<b>Bấm nút cần học trên remote...</b>';
 learnView('waiting');buzz(20);clearInterval(poll);poll=setInterval(pollLearn,500)}
async function cancelLearn(){clearInterval(poll);await api('learn/cancel',{});learnView('idle')}
function learnView(s){const m={idle:'lIdle',waiting:'lWait',captured:'lCap',timeout:'lTo'};
 Object.values(m).forEach(id=>$(id).classList.toggle('on',id==m[s]))}
async function pollLearn(){let s;try{s=await api('learn/status')}catch(e){return}
 if(s.state=='waiting'){$('lRemain').textContent=s.remain;return}
 clearInterval(poll);buzz([40,40,40]);
 if(s.state=='captured'&&s.mode=='ac'){
  if(s.acSupported){toast('✅ Nhận diện: '+s.proto);learnView('idle');show('ac')}
  else{toast('⚠️ Không nhận diện được hãng, hãy học từng nút');showCap(s)}return}
 if(s.state=='captured')showCap(s);else learnView(s.state)}
function showCap(s){learnView('captured');fillDevs();$('lProto').textContent=s.proto+' · '+s.bits+' bit ⓘ';lastDesc=s.desc+'\nRaw: '+s.raw+' xung';
 $('lAcHint').style.display=s.acSupported?'':'none';$('lDev').value=lastDev||curDev||'';$('lName').value=''}
function descSheet(){openSheet(`<h3>Chi tiết tín hiệu</h3><pre>${esc(lastDesc)}</pre><button class="b sec" onclick="closeSheet()">Đóng</button>`)}
async function saveLearn(next){const n=$('lName').value.trim(),d=$('lDev').value.trim();if(!n){toast('Nhập tên nút');$('lName').focus();return}
 await api('learn/save',{n,d});lastDev=d;curDev=d||'Khác';store.set('dev',curDev);buttons.push({id:0,n,d});$('lName').blur();toast('💾 Đã lưu "'+n+'"');
 if(next)startLearn('btn');else{learnView('idle');show('remote')}}
$('lDev').addEventListener('keydown',e=>{if(e.key=='Enter')$('lName').focus()});
$('lName').addEventListener('keydown',e=>{if(e.key=='Enter')saveLearn(true)});

// ---- Điều hòa ----
const MODES=[[0,'🔄','Tự động'],[1,'❄️','Lạnh'],[3,'💧','Khô'],[4,'🌀','Quạt'],[2,'🔥','Sưởi']];
const FANS=[[0,'🅰️','Tự động'],[2,'▂','Thấp'],[3,'▃▅','Vừa'],[4,'▂▅▇','Cao'],[5,'💨','Max']];
const SWINGS=[[-1,'⏸','Tắt'],[0,'↕️','Tự động'],[1,'⬆️','Cao'],[3,'➡️','Giữa'],[5,'⬇️','Thấp']];
const EXTRA=[['turbo','🚀','Turbo'],['quiet','🤫','Êm'],['econo','🌱','Tiết kiệm'],['light','💡','Đèn']];
const chips=(list,key)=>list.map(([v,i,t])=>`<button class="${ac[key]==v?'on':''}" onclick="acSet({${key}:${v}})"><i>${i}</i>${t}</button>`).join('');
function renderAc(){$('acT').textContent=ac.temp+'°';
 $('tempBox').className='tempbox '+(ac.power?'m'+ac.mode:'off');
 $('acSub').textContent=ac.power?(MODES.find(m=>m[0]==ac.mode)||['','',''])[2]:'Đang tắt';
 $('acPow').classList.toggle('on',!!ac.power);
 $('acMode').innerHTML=chips(MODES,'mode');$('acFan').innerHTML=chips(FANS,'fan');$('acSwing').innerHTML=chips(SWINGS,'swing');
 $('acExtra').innerHTML=EXTRA.map(([k,i,t])=>`<button class="${ac[k]?'on':''}" onclick="acSet({${k}:${ac[k]?0:1}})"><i>${i}</i>${t}</button>`).join('');
 $('acBrand').textContent=ac.proto+(ac.model>0?' · Model '+ac.model:'')}
async function loadAc(){const r=await api('ac');ac=r.state;acProtos=r.protocols;renderAc()}
async function acSet(p){buzz(20);const r=await api('ac',p);ac=r.state;renderAc();if(p.send!==0)toast('📤 Đã gửi lệnh')}
function acTemp(d){buzz(15);ac.temp=Math.min(32,Math.max(16,ac.temp+d));renderAc();clearTimeout(acTimer);acTimer=setTimeout(()=>acSet({temp:ac.temp}),400)}
function brandSheet(){openSheet(`<h3>Hãng điều hòa</h3>
 <button class="b" style="margin:10px 0" onclick="closeSheet();startLearn('ac')">🔍 Tự dò hãng (bấm 1 nút trên remote)</button>
 <label>Hoặc chọn thủ công</label><select id="bP">${acProtos.map(p=>`<option${p==ac.proto?' selected':''}>${p}</option>`).join('')}</select>
 <label>Model (-1 = mặc định; thử 1, 2, 3 nếu không ăn lệnh)</label><input id="bM" type="number" inputmode="numeric" value="${ac.model}">
 <div class="row" style="margin-top:16px"><button class="b sec" onclick="closeSheet()">Hủy</button><button class="b" onclick="saveBrand()">Lưu & gửi thử</button></div>`)}
async function saveBrand(){await acSet({proto:$('bP').value,model:$('bM').value});closeSheet()}

// ---- Cài đặt / WiFi ----
const bars=r=>r>-55?'▂▄▆█':r>-67?'▂▄▆':r>-78?'▂▄':'▂';
async function loadInfo(){const i=await api('info');updNet(i);
 $('staTag').textContent={off:'Chưa cài',connecting:'Đang kết nối…',connected:'✅ Đã kết nối',failed:'❌ Lỗi kết nối'}[i.sta];
 $('staBox').innerHTML=i.staSsid?`<span>Mạng</span><b>${esc(i.staSsid)}</b>${i.staIp?`<span>Địa chỉ</span><b>${i.host}</b><span>IP</span><b>${i.staIp} (${i.rssi} dBm)</b>`:''}`:'';
 $('staHelp').style.display=i.staSsid?'none':'';$('forgetBtn').style.display=i.staSsid?'':'none';
 $('info').innerHTML=`<span>WiFi phát</span><b>${esc(i.apSsid)} (mở)</b><span>Địa chỉ</span><b>${i.apIp}</b><span>Đang kết nối</span><b>${i.apClients} thiết bị</b>
  <span>RAM trống</span><b>${(i.heap/1024).toFixed(1)} KB</b><span>Bộ nhớ</span><b>${(i.fsUsed/1024).toFixed(0)} / ${(i.fsTotal/1024).toFixed(0)} KB</b>
  <span>Thời gian chạy</span><b>${Math.floor(i.uptime/3600)}h ${Math.floor(i.uptime%3600/60)}m</b>`;
 if(i.sta=='connecting')setTimeout(()=>$('set').classList.contains('on')&&loadInfo(),2000)}
function updNet(i){$('net').innerHTML=i.sta=='connected'?'<span class="dot ok"></span>'+esc(i.staSsid):'<span class="dot warn"></span>'+esc(i.apSsid)}
async function scanSheet(){openSheet('<h3>Chọn WiFi</h3><p class="muted center" style="padding:24px 0">⏳ Đang tìm mạng...</p>');let r={};
 for(let k=0;k<20;k++){r=await api('wifi/scan').catch(()=>({}));if(!r.scanning)break;await new Promise(z=>setTimeout(z,700))}
 const l=(r.list||[]).sort((a,b)=>b.rssi-a.rssi);
 openSheet(`<h3>Chọn WiFi</h3><div class="list">${l.length?l.map(w=>`<button data-s="${esc(w.ssid)}" data-l="${w.lock?1:0}"><span>${esc(w.ssid)}</span><span class="muted">${w.lock?'🔒 ':''}${bars(w.rssi)}</span></button>`).join(''):'<p class="muted">Không tìm thấy mạng nào</p>'}</div>
  <div class="row" style="margin-top:12px"><button class="b sec" onclick="closeSheet()">Đóng</button><button class="b sec" onclick="scanSheet()">🔄 Tìm lại</button></div>`);
 $('sheet').querySelectorAll('.list button').forEach(b=>b.onclick=()=>wifiSheet(b.dataset.s,b.dataset.l=='1'))}
function wifiSheet(ssid,lock){openSheet(`<h3>${esc(ssid)}</h3>${lock?`<label>Mật khẩu</label><input id="wP" type="password" autocomplete="off" enterkeyhint="go">
 <label style="display:flex;gap:8px;align-items:center"><input type="checkbox" style="width:auto" onchange="$('wP').type=this.checked?'text':'password'">Hiện mật khẩu</label>`:'<p class="muted">Mạng không có mật khẩu</p>'}
 <div class="row" style="margin-top:16px"><button class="b sec" onclick="closeSheet()">Hủy</button><button class="b" id="wGo">Kết nối</button></div>`);
 const go=async()=>{await api('wifi/save',{ssid,pass:lock?$('wP').value:''});closeSheet();toast('Đang kết nối '+ssid+'...');loadInfo()};
 $('wGo').onclick=go;if(lock){$('wP').onkeydown=e=>{if(e.key=='Enter')go()};setTimeout(()=>$('wP').focus(),300)}}
function forget(){openSheet(`<h3>Quên WiFi nhà?</h3><p class="muted">Thiết bị vẫn phát WiFi "IR-Remote" để điều khiển.</p>
 <div class="row" style="margin-top:12px"><button class="b sec" onclick="closeSheet()">Hủy</button><button class="b bad" onclick="api('wifi/forget',{}).then(()=>{closeSheet();loadInfo()})">Quên</button></div>`)}

api('info').then(updNet).catch(()=>{});
loadButtons();
</script></body></html>)HTML";

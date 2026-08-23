#pragma once

#include <Arduino.h>

// ======================================================================
// --- WEB INTERFACE HTML (Miu Controller) ---
// ======================================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Miu Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <meta charset="UTF-8">
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap');

    :root {
      --accent: #a855f7;
      --accent-dark: #9333ea;
      --accent-glow: rgba(168,85,247,0.35);
      --accent2: #38bdf8;
      --accent2-glow: rgba(56,189,248,0.25);
      --bg-layer1: rgba(14,9,34,0.78);
      --bg-layer2: rgba(24,14,52,0.68);
      --glass-border: rgba(168,85,247,0.2);
      --text-primary: #f0e8ff;
      --text-secondary: #94a3b8;
      --text-muted: #64748b;
    }

    *{user-select:none;-webkit-user-select:none;-webkit-touch-callout:none;box-sizing:border-box;}

    body {
      font-family: 'Inter','Segoe UI',Roboto,sans-serif;
      text-align: center;
      background:
        radial-gradient(ellipse 80% 60% at 20% 10%, rgba(168,85,247,0.13) 0%, transparent 60%),
        radial-gradient(ellipse 60% 50% at 80% 80%, rgba(56,189,248,0.10) 0%, transparent 55%),
        linear-gradient(160deg,#050510 0%,#0a0520 45%,#060518 100%);
      background-attachment: fixed;
      color: var(--text-primary);
      touch-action: manipulation;
      margin: 0; padding: 10px;
      min-height: 100vh; overflow-x: hidden;
    }
    body::before{content:'';position:fixed;top:-20%;left:-10%;width:55%;height:55%;background:radial-gradient(circle,rgba(168,85,247,0.07) 0%,transparent 70%);border-radius:50%;animation:floatOrb 18s ease-in-out infinite alternate;pointer-events:none;z-index:0;}
    body::after{content:'';position:fixed;bottom:-15%;right:-10%;width:50%;height:50%;background:radial-gradient(circle,rgba(56,189,248,0.06) 0%,transparent 70%);border-radius:50%;animation:floatOrb 22s ease-in-out infinite alternate-reverse;pointer-events:none;z-index:0;}
    @keyframes floatOrb{from{transform:translate(0,0) scale(1);}to{transform:translate(4%,3%) scale(1.08);}}

    h2{margin:10px 0 4px 0;font-size:27px;font-weight:700;letter-spacing:-0.5px;background:linear-gradient(120deg,#c084fc 0%,#38bdf8 60%,#e879f9 100%);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;position:relative;z-index:1;filter:drop-shadow(0 0 16px rgba(168,85,247,0.4));}

    /* --- Battery Bar --- */
    .battery-row{display:flex;align-items:center;justify-content:center;gap:7px;margin:0 0 8px 0;position:relative;z-index:1;}
    .battery-shell{width:120px;height:17px;border:1.5px solid rgba(168,85,247,0.3);border-radius:5px;background:rgba(8,4,20,0.7);overflow:hidden;position:relative;}
    .battery-shell::after{content:'';position:absolute;right:-6px;top:50%;transform:translateY(-50%);width:4px;height:8px;background:rgba(168,85,247,0.35);border-radius:0 2px 2px 0;}
    .battery-fill{height:100%;border-radius:3px 0 0 3px;transition:width .6s ease,background .6s ease;width:0%;background:linear-gradient(90deg,#22c55e,#4ade80);}
    .hunger-fill{height:100%;border-radius:3px 0 0 3px;transition:width .6s ease,background .6s ease;width:0%;background:linear-gradient(90deg,#eab308,#facc15);}
    .love-fill{height:100%;border-radius:3px 0 0 3px;transition:width .6s ease,background .6s ease;width:0%;background:linear-gradient(90deg,#ec4899,#f472b6);}
    .battery-pct{font-size:12px;font-weight:600;color:var(--text-secondary);min-width:36px;text-align:left;}

    /* --- Queue --- */
    .cmd-queue{font-size:11px;color:var(--text-muted);margin-bottom:12px;letter-spacing:.5px;position:relative;z-index:1;}
    .cmd-queue.full{color:#f87171;font-weight:600;}

    /* --- Layout --- */
    .sections{display:flex;flex-direction:column;gap:12px;max-width:1400px;margin:0 auto;position:relative;z-index:1;}

    .section{background:var(--bg-layer1);border:1px solid var(--glass-border);border-radius:20px;padding:14px;margin:0 auto;width:calc(100% - 14px);max-width:460px;box-shadow:0 4px 24px rgba(0,0,0,.5),inset 0 1px 0 rgba(255,255,255,.06);backdrop-filter:blur(16px) saturate(160%);-webkit-backdrop-filter:blur(16px) saturate(160%);transition:box-shadow .3s,border-color .3s;}
    .section:hover{border-color:rgba(168,85,247,.32);}
    .sec-title{font-size:10px;font-weight:600;color:var(--accent);margin:0 0 11px 0;text-transform:uppercase;letter-spacing:2px;}

    /* --- Buttons base --- */
    button{background:var(--bg-layer2);border:1px solid rgba(168,85,247,.18);color:var(--text-primary);padding:13px;font-size:15px;border-radius:13px;cursor:pointer;box-shadow:0 2px 8px rgba(0,0,0,.4),inset 0 1px 0 rgba(255,255,255,.05);transition:all .15s ease;font-weight:500;font-family:inherit;backdrop-filter:blur(8px);}
    button:hover{background:rgba(168,85,247,.12);border-color:rgba(168,85,247,.38);box-shadow:0 4px 16px rgba(168,85,247,.18);transform:translateY(-1px);}
    button:active{transform:translateY(1px);background:rgba(168,85,247,.22);}

    /* --- D-Pad --- */
    .dpad-wrap{display:flex;flex-direction:column;align-items:center;gap:10px;}
    .dpad{display:grid;grid-template-columns:repeat(3,1fr);grid-template-rows:repeat(2,1fr);gap:9px;width:100%;max-width:280px;aspect-ratio:3/2;}
    .dpad button{font-size:28px;border:1px solid rgba(56,189,248,.22);color:#7dd3fc;width:100%;height:100%;min-height:62px;background:rgba(12,26,48,.7);border-radius:15px;text-shadow:0 0 10px rgba(56,189,248,.5);padding:0;}
    .dpad button:hover{background:rgba(56,189,248,.12);border-color:rgba(56,189,248,.48);}
    .dpad button:active{background:rgba(56,189,248,.25);}
    .spacer{visibility:hidden;}

    /* --- Joystick --- */
    #joystickWrap{display:flex;flex-direction:column;align-items:center;gap:8px;}
    #joystickCanvas{display:block;border-radius:50%;}

    /* --- Tilt Section --- */
    #tiltWrap{display:flex;flex-direction:column;align-items:center;gap:12px;padding:14px 0;}
    #tiltArrows{display:grid;grid-template-columns:1fr 1fr 1fr;grid-template-rows:1fr 1fr 1fr;gap:4px;width:108px;height:108px;}
    .t-arrow{display:flex;align-items:center;justify-content:center;font-size:28px;border-radius:10px;background:rgba(168,85,247,.08);border:1.5px solid rgba(168,85,247,.18);color:rgba(168,85,247,.25);transition:all .15s ease;user-select:none;}
    .t-arrow.t-active{background:rgba(168,85,247,.35);border-color:rgba(168,85,247,.8);color:#e9d5ff;box-shadow:0 0 18px rgba(168,85,247,.55);transform:scale(1.13);}
    .t-center{background:rgba(168,85,247,.04);border:1.5px solid rgba(168,85,247,.12);border-radius:10px;}
    .t-empty{background:transparent;border:none;}

    /* --- Stop button --- */
    .btn-stop{background:linear-gradient(145deg,rgba(239,68,68,.28),rgba(185,28,28,.22));border:1px solid rgba(239,68,68,.38);width:100%;font-size:16px;padding:13px;color:#fca5a5;text-transform:uppercase;letter-spacing:2.5px;font-weight:700;border-radius:15px;text-shadow:0 0 10px rgba(239,68,68,.5);}
    .btn-stop:hover{background:linear-gradient(145deg,rgba(239,68,68,.42),rgba(185,28,28,.35));box-shadow:0 0 22px rgba(239,68,68,.28);}

    /* --- Pose Grid --- */
    .pose-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;}
    .btn-pose{background:linear-gradient(145deg,rgba(168,85,247,.2),rgba(147,51,234,.13));border:1px solid rgba(168,85,247,.28);color:#e9d5ff;padding:9px 5px;font-size:13px;border-radius:11px;text-shadow:0 0 8px rgba(168,85,247,.4);}
    .btn-pose:hover{background:linear-gradient(145deg,rgba(168,85,247,.36),rgba(147,51,234,.26));box-shadow:0 0 15px rgba(168,85,247,.2);}
    .btn-pose:active{background:linear-gradient(145deg,rgba(168,85,247,.5),rgba(126,34,206,.4));}

    /* --- Face Grid --- */
    .face-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:7px;}
    .btn-face{background:linear-gradient(145deg,rgba(56,189,248,.1),rgba(14,165,233,.07));border:1px solid rgba(56,189,248,.2);color:#bae6fd;padding:8px 4px;font-size:11px;border-radius:11px;line-height:1.3;}
    .btn-face:hover{background:linear-gradient(145deg,rgba(56,189,248,.24),rgba(14,165,233,.17));border-color:rgba(56,189,248,.45);}
    .btn-face:active{background:linear-gradient(145deg,rgba(56,189,248,.38),rgba(14,165,233,.3));}
    .fe{display:block;font-size:20px;margin-bottom:2px;}

    /* --- System Section --- */
    .btn-settings{background:rgba(28,18,58,.7);border:1px solid rgba(168,85,247,.25);color:#c4b5fd;padding:9px 18px;font-size:13px;border-radius:11px;}
    .btn-settings:hover{background:rgba(168,85,247,.15);border-color:rgba(168,85,247,.52);}
    .gamepad-status{font-size:11px;padding:6px 12px;border-radius:9px;border:1px solid rgba(168,85,247,.2);color:var(--text-muted);background:rgba(8,4,20,.5);display:inline-block;}
    .gamepad-status.connected{border-color:rgba(56,189,248,.4);color:#7dd3fc;background:rgba(56,189,248,.08);}

    /* --- Touch status --- */
    .touch-row{display:flex;align-items:center;gap:7px;margin-top:9px;font-size:11px;justify-content:center;}
    .t-dot{width:9px;height:9px;border-radius:50%;background:#334155;transition:background .3s,box-shadow .3s;flex-shrink:0;}
    .t-dot.on{background:#38bdf8;box-shadow:0 0 9px #38bdf8;}

    /* --- Mode Pills --- */
    .mode-pills{display:flex;gap:5px;margin-top:9px;}
    .m-pill{flex:1;padding:7px 3px;font-size:11px;border-radius:8px;border:1px solid rgba(168,85,247,.2);background:rgba(22,12,48,.6);color:var(--text-muted);cursor:pointer;transition:all .2s;font-family:inherit;font-weight:500;}
    .m-pill.on{background:rgba(168,85,247,.25);border-color:rgba(168,85,247,.62);color:#e9d5ff;box-shadow:0 0 12px rgba(168,85,247,.2);}

    /* --- Speech & Melody --- */
    .speech-inp{width:100%;background:rgba(9,4,28,.8);border:1px solid rgba(168,85,247,.25);color:var(--text-primary);padding:10px;border-radius:9px;font-size:13px;margin-bottom:8px;outline:none;}
    .speech-inp:focus{border-color:rgba(56,189,248,.5);}
    .piano{display:flex;gap:2px;margin-bottom:8px;}
    .pkey{flex:1;height:40px;background:var(--bg-layer2);border:1px solid rgba(168,85,247,.2);border-radius:4px;cursor:pointer;display:flex;align-items:center;justify-content:center;font-size:12px;font-weight:600;}
    .pkey:hover{background:rgba(168,85,247,.2);}
    .pkey:active{background:rgba(168,85,247,.4);box-shadow:inset 0 0 10px rgba(168,85,247,.5);}

    /* --- Settings Overlay --- */
    .settings-panel{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(3,2,15,.84);z-index:100;backdrop-filter:blur(18px) saturate(140%);-webkit-backdrop-filter:blur(18px) saturate(140%);overflow-y:auto;}
    .settings-content{background:linear-gradient(160deg,rgba(17,9,43,.96) 0%,rgba(9,5,28,.98) 100%);border:1px solid rgba(168,85,247,.25);max-width:420px;margin:22px auto;padding:22px;border-radius:22px;text-align:left;box-shadow:0 20px 60px rgba(0,0,0,.72),inset 0 1px 0 rgba(255,255,255,.04);}
    .settings-content h3{color:transparent;background:linear-gradient(90deg,#c084fc,#38bdf8);-webkit-background-clip:text;background-clip:text;margin-top:0;text-align:center;font-size:20px;font-weight:700;}
    .set-section{margin:13px 0;padding:12px;background:rgba(168,85,247,.05);border-radius:12px;border:1px solid rgba(168,85,247,.12);}
    .set-section h4{color:var(--accent2);margin:0 0 9px 0;font-size:11px;text-transform:uppercase;letter-spacing:1.5px;font-weight:600;}
    .settings-content label{display:block;margin-top:9px;font-weight:500;color:var(--text-secondary);font-size:12px;}
    .settings-content input,.settings-content select{width:100%;padding:8px 10px;margin-top:4px;background:rgba(9,4,28,.8);color:var(--text-primary);border:1px solid rgba(168,85,247,.25);border-radius:9px;font-size:12px;font-family:inherit;outline:none;}
    .settings-content input:focus,.settings-content select:focus{border-color:rgba(168,85,247,.6);}
    .settings-content select option{background:#0d0822;}
    .thr-row{display:flex;align-items:center;gap:9px;margin-top:7px;}
    .thr-row input[type="range"]{flex:1;height:5px;background:rgba(56,189,248,.15);border-radius:5px;outline:none;-webkit-appearance:none;border:1px solid rgba(56,189,248,.22);}
    .thr-row input[type="range"]::-webkit-slider-thumb{-webkit-appearance:none;width:17px;height:17px;background:linear-gradient(135deg,#38bdf8,#a855f7);border-radius:50%;cursor:pointer;}
    .thr-badge{font-size:12px;font-weight:600;color:var(--accent2);min-width:50px;text-align:right;}
    .btn-save{background:linear-gradient(135deg,rgba(56,189,248,.22),rgba(168,85,247,.18));border:1px solid rgba(56,189,248,.33);color:#bae6fd;width:100%;margin-top:18px;font-weight:600;letter-spacing:.5px;border-radius:11px;}
    .btn-save:hover{background:linear-gradient(135deg,rgba(56,189,248,.36),rgba(168,85,247,.28));box-shadow:0 0 18px rgba(56,189,248,.22);}
    .btn-close{background:rgba(239,68,68,.1);border:1px solid rgba(239,68,68,.22);color:#fca5a5;width:100%;margin-top:8px;border-radius:11px;}
    .btn-close:hover{background:rgba(239,68,68,.2);border-color:rgba(239,68,68,.45);}

    /* --- Motor panel --- */
    .lock-ind{font-size:10px;color:#f87171;text-align:center;margin-top:4px;display:none;}
    .lock-ind.on{display:block;}
    .mslider{margin:11px 0;}
    .mslider label{display:flex;justify-content:space-between;font-size:11px;color:var(--text-secondary);margin-bottom:4px;font-weight:500;}
    .mslider input[type="range"]{width:100%;height:5px;background:rgba(168,85,247,.15);border-radius:5px;outline:none;-webkit-appearance:none;border:1px solid rgba(168,85,247,.2);}
    .mslider input[type="range"]::-webkit-slider-thumb{-webkit-appearance:none;width:17px;height:17px;background:linear-gradient(135deg,var(--accent),var(--accent2));border-radius:50%;cursor:pointer;box-shadow:0 0 7px rgba(168,85,247,.5);}
    .mslider input[type="range"]::-moz-range-thumb{width:17px;height:17px;background:linear-gradient(135deg,var(--accent),var(--accent2));border-radius:50%;cursor:pointer;border:none;}
    .mslider input[type="range"]:disabled{opacity:.4;}

    /* --- Pulse animation --- */
    @keyframes touchPulse{0%{box-shadow:0 0 0 0 rgba(56,189,248,.6);}70%{box-shadow:0 0 0 18px rgba(56,189,248,0);}100%{box-shadow:0 0 0 0 rgba(56,189,248,0);}}
    .tpulse{animation:touchPulse .6s ease-out;}

    /* --- Desktop layout --- */
    @media(min-width:1024px){
      body{padding:16px;}
      .section{padding:20px;width:100%;}
      h2{font-size:32px;}
      .sections{flex-direction:row;justify-content:center;align-items:flex-start;gap:40px;padding:0 18px;}
      .sec-col{flex:0 1 460px;display:flex;flex-direction:column;gap:14px;}
      .section{width:100%;max-width:460px;margin:0;}
    }
  </style>
</head>
<body>

<h2>&#x1F43E; Miu Controller</h2>

<!-- Battery Bar -->
<div class="battery-row">
  <span style="font-size:13px;">&#128267;</span>
  <div class="battery-shell"><div class="battery-fill" id="batFill"></div></div>
  <span class="battery-pct" id="batPct">---%</span>
</div>

<!-- Hunger Bar -->
<div class="battery-row" style="margin-bottom:4px;">
  <span style="font-size:13px;">&#127828;</span>
  <div class="battery-shell"><div class="hunger-fill" id="hunFill"></div></div>
  <span class="battery-pct" id="hunPct">---%</span>
</div>

<!-- Love Bar -->
<div class="battery-row" style="margin-bottom:12px;">
  <span style="font-size:13px;">&#128149;</span>
  <div class="battery-shell"><div class="love-fill" id="loveFill"></div></div>
  <span class="battery-pct" id="lovePct">---%</span>
</div>

<div class="cmd-queue" id="qStatus">Command Queue: 0/3</div>

<div class="sections">
  <div class="sec-col">

    <!-- MOVEMENT CONTROL -->
    <div class="section">
      <div class="sec-title">Movement Control</div>

      <!-- D-Pad -->
      <div id="dpadWrap" style="display:none;">
        <div class="dpad-wrap">
          <div class="dpad">
            <div class="spacer"></div>
            <button onmousedown="jmove('forward')" onmouseup="jstop()" ontouchstart="jmove('forward')" ontouchend="jstop()">&#9650;</button>
            <div class="spacer"></div>
            <button onmousedown="jmove('left')" onmouseup="jstop()" ontouchstart="jmove('left')" ontouchend="jstop()">&#9664;</button>
            <button onmousedown="jmove('backward')" onmouseup="jstop()" ontouchstart="jmove('backward')" ontouchend="jstop()">&#9660;</button>
            <button onmousedown="jmove('right')" onmouseup="jstop()" ontouchstart="jmove('right')" ontouchend="jstop()">&#9654;</button>
          </div>
        </div>
      </div>

      <!-- Virtual Joystick -->
      <div id="joystickWrap">
        <canvas id="joystickCanvas" width="230" height="230" style="touch-action:none;display:block;"></canvas>
        <div style="font-size:10px;color:var(--text-muted);">Tutun &amp; s&uuml;r&uuml;kleyin &mdash; b&#305;rak&#305;nca durur</div>
      </div>

      <!-- Tilt -->
      <div id="tiltWrap" style="display:none;">
        <div style="font-size:32px;margin-bottom:2px;">&#128241;</div>
        <div style="font-size:11px;color:var(--text-muted);text-align:center;line-height:1.6;margin-bottom:8px;">Telefonu e&#287;erek robotu y&#246;nlendir</div>
        
        <!-- Avatar / Drive Toggle -->
        <div style="display:flex; justify-content:center; margin-bottom:12px;">
          <div class="mode-pills">
            <button class="m-pill on" id="pill-tilt-drive" onclick="setTiltMode('drive')">S&#252;r&#252;&#351;</button>
            <button class="m-pill" id="pill-tilt-avatar" onclick="setTiltMode('avatar')">Avatar</button>
          </div>
        </div>

        <!-- Direction Arrow Grid -->
        <div id="tiltArrows">
          <div class="t-empty"></div>
          <div class="t-arrow" id="ta-forward">&#8593;</div>
          <div class="t-empty"></div>
          <div class="t-arrow" id="ta-left">&#8592;</div>
          <div class="t-center"></div>
          <div class="t-arrow" id="ta-right">&#8594;</div>
          <div class="t-empty"></div>
          <div class="t-arrow" id="ta-backward">&#8595;</div>
          <div class="t-empty"></div>
        </div>
        <div id="tiltStatus" style="font-size:12px;color:var(--accent2);font-weight:600;margin-top:4px;">Sen&#246;r ba&#287;lan&#305;yor...</div>
        <div id="tiltDbg" style="font-size:10px;color:var(--text-muted);margin-top:2px;font-family:monospace;"></div>
        <div id="tiltErr" style="font-size:11px;color:#f87171;margin-top:4px;text-align:center;display:none;"></div>
        <button id="tiltPermBtn" onclick="reqTiltPerm()" class="btn-settings" style="display:none;margin-top:8px;">iOS &#304;zni Ver</button>
      </div>

      <button class="btn-stop" onclick="stop()" style="margin-top:11px;">&#9632; STOP ALL</button>
    </div>

    <!-- POSES & ANIMATIONS -->
    <div class="section">
      <div class="sec-title">Poses &amp; Animations</div>
      <div class="pose-grid">
        <button class="btn-pose" onclick="pose('rest')">Rest</button>
        <button class="btn-pose" onclick="pose('cat rest')">Cat Rest</button>
        <button class="btn-pose" onclick="pose('stand')">Stand</button>
        <button class="btn-pose" onclick="pose('wave')">Wave</button>
        <button class="btn-pose" onclick="pose('dance')">Dance</button>
        <button class="btn-pose" onclick="pose('swim')">Swim</button>
        <button class="btn-pose" onclick="pose('point')">Point</button>
        <button class="btn-pose" onclick="pose('pushup')">Pushup</button>
        <button class="btn-pose" onclick="pose('bow')">Bow</button>
        <button class="btn-pose" onclick="pose('cute')">Cute</button>
        <button class="btn-pose" onclick="pose('freaky')">Freaky</button>
        <button class="btn-pose" onclick="pose('worm')">Worm</button>
        <button class="btn-pose" onclick="pose('shake')">Shake</button>
        <button class="btn-pose" onclick="pose('shrug')">Shrug</button>
        <button class="btn-pose" onclick="pose('dead')">Dead</button>
        <button class="btn-pose" onclick="pose('crab')">Crab</button>
        <button class="btn-pose" onclick="pose('knead')">Knead 🐾</button>
      </div>
    </div>

    <!-- FACE SELECTOR -->
    <div class="section">
      <div class="sec-title">&#x1F43E; Miu Y&uuml;z Se&ccedil;ici</div>
      <div class="face-grid">
        <button class="btn-face" onclick="setFaceCmd('happy')"><span class="fe">&#128522;</span>Happy</button>
        <button class="btn-face" onclick="setFaceCmd('sad')"><span class="fe">&#128546;</span>Sad</button>
        <button class="btn-face" onclick="setFaceCmd('angry')"><span class="fe">&#128544;</span>Angry</button>
        <button class="btn-face" onclick="setFaceCmd('surprised')"><span class="fe">&#128562;</span>Surprised</button>
        <button class="btn-face" onclick="setFaceCmd('sleepy')"><span class="fe">&#128564;</span>Sleepy</button>
        <button class="btn-face" onclick="setFaceCmd('love')"><span class="fe">&#129392;</span>Love</button>
        <button class="btn-face" onclick="setFaceCmd('excited')"><span class="fe">&#129321;</span>Excited</button>
        <button class="btn-face" onclick="setFaceCmd('confused')"><span class="fe">&#129300;</span>Confused</button>
        <button class="btn-face" onclick="setFaceCmd('thinking')"><span class="fe">&#128161;</span>Thinking</button>
        <button class="btn-face" onclick="setFaceCmd('idle')"><span class="fe">&#128528;</span>Idle</button>
        <button class="btn-face" onclick="setFaceCmd('rest')"><span class="fe">&#128524;</span>Rest</button>
        <button class="btn-face" onclick="setFaceCmd('default')"><span class="fe">&#128528;</span>Default</button>
      </div>
    </div>

  </div><!-- /sec-col left -->

  <div class="sec-col">

    <!-- PET INTERACTIONS -->
    <div class="section">
      <div class="sec-title">&#128062; Evcil Hayvan Etkile&#351;imi</div>
      <div style="display:flex;gap:8px;margin-bottom:8px;">
        <button class="btn-face" style="flex:1;padding:12px;font-size:13px;background:rgba(34,197,94,.1);border-color:rgba(34,197,94,.3);color:#bbf7d0;" onclick="feedMiu(event)">&#127850; Besle</button>
        <button class="btn-face" style="flex:1;padding:12px;font-size:13px;background:rgba(236,72,153,.1);border-color:rgba(236,72,153,.3);color:#fbcfe8;" onclick="tickleMiu(event)">&#129505; G&ıd&ıkla</button>
        <button class="btn-face" style="flex:1;padding:12px;font-size:13px;background:rgba(56,189,248,.1);border-color:rgba(56,189,248,.3);color:#bae6fd;" onclick="waterMiu(event)">&#128167; Su &#304;&ccedil;ir</button>
      </div>
      <button class="btn-settings" style="width:100%;background:rgba(56,189,248,.15);border-color:rgba(56,189,248,.4);color:#bae6fd;" onclick="findMiu(event)">&#128205; Neredesin Miu?</button>
    </div>

    <!-- EMOJI MIRROR -->
    <div class="section">
      <div class="sec-title">&#129505; Emoji Ayna Modu</div>
      <p style="font-size:11px;color:var(--text-secondary);margin:0 0 8px;">Bir duygu sec, Miu da ayn&#305;s&#305;n&#305; taklit etsin!</p>
      <div style="display:grid;grid-template-columns:repeat(3,1fr);gap:6px;">
        <button class="btn-face" style="font-size:18px;padding:10px 4px;" onclick="sendEmoji('happy')">&#128512;</button>
        <button class="btn-face" style="font-size:18px;padding:10px 4px;" onclick="sendEmoji('sad')">&#128546;</button>
        <button class="btn-face" style="font-size:18px;padding:10px 4px;" onclick="sendEmoji('angry')">&#128545;</button>
        <button class="btn-face" style="font-size:18px;padding:10px 4px;" onclick="sendEmoji('excited')">&#129321;</button>
        <button class="btn-face" style="font-size:18px;padding:10px 4px;" onclick="sendEmoji('sleepy')">&#128564;</button>
        <button class="btn-face" style="font-size:18px;padding:10px 4px;" onclick="sendEmoji('love')">&#129392;</button>
      </div>
    </div>

    <!-- JUKEBOX -->
    <div class="section">
      <div class="sec-title">&#127926; Jukebox</div>
      <div style="display:grid;grid-template-columns:repeat(2,1fr);gap:6px;">
        <button class="btn-face" style="background:rgba(239,68,68,.1);border-color:rgba(239,68,68,.3);color:#fca5a5;" onclick="playJukebox('mario', this)">&#127909; Mario</button>
        <button class="btn-face" style="background:rgba(251,191,36,.1);border-color:rgba(251,191,36,.3);color:#fde68a;" onclick="playJukebox('nokia', this)">&#128222; Nokia</button>
        <button class="btn-face" style="background:rgba(52,211,153,.1);border-color:rgba(52,211,153,.3);color:#a7f3d0;" onclick="playJukebox('birthday', this)">&#127874; Dogum Gunu</button>
        <button class="btn-face" style="background:rgba(168,85,247,.1);border-color:rgba(168,85,247,.3);color:#e9d5ff;" onclick="playJukebox('starwars', this)">&#11088; Star Wars</button>
        <button class="btn-face" style="background:rgba(56,189,248,.1);border-color:rgba(56,189,248,.3);color:#bae6fd;grid-column:1/-1;" onclick="playJukebox('tetris', this)">&#127025; Tetris</button>
      </div>
    </div>

    <!-- PIXEL ART CUSTOM FACE -->
    <div class="section">
      <div class="sec-title">&#127912; Pixel Art Y&uuml;z</div>
      <div style="display:flex;justify-content:center;margin-bottom:8px;">
        <canvas id="pixelCanvas" width="128" height="64" style="background:#0a0520;border:1px solid rgba(168,85,247,0.4);border-radius:8px;image-rendering:pixelated;width:100%;max-width:256px;touch-action:none;cursor:crosshair;"></canvas>
      </div>
      <div style="display:flex;gap:8px;margin-bottom:8px;">
        <button class="btn-face" style="flex:1;background:rgba(168,85,247,.25);" id="btnPen" onclick="setPixelTool('pen')">&#9998; Kalem</button>
        <button class="btn-face" style="flex:1;" id="btnEraser" onclick="setPixelTool('eraser')">&#129529; Silgi</button>
        <button class="btn-face" style="flex:1;background:rgba(239,68,68,.1);border-color:rgba(239,68,68,.3);color:#fca5a5;" onclick="clearPixelCanvas()">Temizle</button>
      </div>
      <button class="btn-settings" style="width:100%;" onclick="sendCustomFace(event)">&#x1F4E4; Miu'ya G&ouml;nder</button>
    </div>

    <!-- SPEECH BUBBLE -->
    <div class="section">
      <div class="sec-title">&#128172; OLED Konu&#351;ma Balonu</div>
      <input type="text" id="speechTxt" class="speech-inp" placeholder="Miu'nun s&ouml;yleyece&#287;i metin..." maxlength="50">
      <button class="btn-settings" style="width:100%;" onclick="sendSpeech()">G&ouml;nder (5sn g&ouml;sterir)</button>
    </div>

    <!-- MELODY EDITOR -->
    <div class="section">
      <div class="sec-title">&#127925; Miu Melodi Edit&ouml;r&uuml;</div>
      <div class="piano">
        <div class="pkey" onclick="addNote(261,150)">C</div>
        <div class="pkey" onclick="addNote(293,150)">D</div>
        <div class="pkey" onclick="addNote(329,150)">E</div>
        <div class="pkey" onclick="addNote(349,150)">F</div>
        <div class="pkey" onclick="addNote(392,150)">G</div>
        <div class="pkey" onclick="addNote(440,150)">A</div>
        <div class="pkey" onclick="addNote(493,150)">B</div>
        <div class="pkey" onclick="addNote(523,150)">C2</div>
      </div>
      <div style="display:flex;gap:8px;margin-bottom:8px;">
        <button class="btn-face" style="flex:1;" onclick="addNote(0,150)">Es (Bo&#351;luk)</button>
        <button class="btn-face" style="flex:1;background:rgba(239,68,68,.1);border-color:rgba(239,68,68,.3);color:#fca5a5;" onclick="clearMelody()">Temizle</button>
      </div>
      <div style="font-size:11px;color:var(--text-muted);margin-bottom:8px;" id="melStr">Bekleniyor... (Max 32 nota)</div>
      <button class="btn-settings" style="width:100%;" onclick="playMelody()">&#9654; Melodiyi &Ccedil;al &amp; Dans Et</button>
    </div>

    <!-- SYSTEM -->
    <div class="section">
      <div class="sec-title">System</div>
      <button class="btn-settings" onclick="openSettings()">&#9881;&#65039; Settings</button>

      <div style="margin-top:11px;">
        <div id="gamepadStatus" class="gamepad-status">Gamepad disconnected</div>
      </div>

      <div class="touch-row">
        <div class="t-dot" id="tDotMain"></div>
        <span id="tStatusTxt" style="color:var(--text-secondary);">Miu'nun kafa sens&ouml;r&uuml;: bekliyor</span>
      </div>

      <!-- Control Mode Pills -->
      <div style="margin-top:12px;">
        <div style="font-size:9px;color:var(--text-muted);margin-bottom:5px;text-transform:uppercase;letter-spacing:1px;">Kontrol Modu</div>
        <div class="mode-pills">
          <button class="m-pill" id="pill-dpad"     onclick="setCtrlMode('dpad')">D-Pad</button>
          <button class="m-pill on" id="pill-joystick" onclick="setCtrlMode('joystick')">Joystick</button>
          <button class="m-pill" id="pill-tilt"     onclick="setCtrlMode('tilt')">Tilt</button>
        </div>
      </div>
    </div>

  </div><!-- /sec-col right -->
</div><!-- /sections -->

<!-- ===== SETTINGS PANEL ===== -->
<div id="settingsPanel" class="settings-panel">
  <div class="settings-content">
    <h3>&#9881; Settings</h3>

    <div class="set-section">
      <h4>Animation Parameters</h4>
      <label>Frame Delay (ms):</label>
      <input type="number" id="frameDelay" min="1" max="1000" step="1">
      <label>Walk Cycles:</label>
      <input type="number" id="walkCycles" min="1" max="50" step="1">
    </div>

    <div class="set-section">
      <h4>Motor Settings</h4>
      <label>Motor Current Delay (ms):</label>
      <input type="number" id="motorCurrentDelay" min="0" max="500" step="1">
      <label>Motor Speed:</label>
      <select id="motorSpeed">
        <option value="slow">Slow</option>
        <option value="medium" selected>Medium</option>
        <option value="fast">Fast</option>
      </select>
    </div>

    <div class="set-section">
      <h4>&#x1F43E; Miu Kafa Sens&ouml;r&uuml; (GPIO 12)</h4>
      <div style="font-size:10px;color:var(--text-muted);margin-top:5px;line-height:1.5;">
        TTP223 dijital dokunmatik sens&ouml;r devrede. (Otomatik Kalibrasyon)
      </div>
      <div style="display:flex;align-items:center;gap:7px;margin-top:8px;font-size:11px;color:var(--text-muted);">
        <div class="t-dot" id="tDotSettings"></div>
      </div>
    </div>

    <div class="set-section">
      <h4>Kontrol Modu</h4>
      <label>Hareket Kontrol&uuml;:</label>
      <select id="ctrlModeSelect" onchange="setCtrlMode(this.value)">
        <option value="joystick" selected>Virtual Joystick</option>
        <option value="dpad">D-Pad</option>
        <option value="tilt">Tilt (Jiroscop)</option>
      </select>
      <div style="margin-top:12px;display:flex;align-items:center;justify-content:space-between;">
        <label style="margin:0;color:var(--text-primary);font-size:12px;">Tam Otonom (Pet) Modu:</label>
        <input type="checkbox" id="autonomousMode" style="width:18px;height:18px;accent-color:var(--accent);">
      </div>
      <div style="font-size:10px;color:var(--text-muted);margin-top:5px;line-height:1.5;">
        A&ccedil;&#305;kken, 30 saniye bo&#351;ta kald&#305;&#287;&#305;nda kendili&287;inden hareketler yapar (5 dk sonra uyur).
      </div>
    </div>

    <div class="set-section">
      <h4>Theme</h4>
      <label>Accent Color:</label>
      <select id="themeColor">
        <option value="purple">Purple / Blue (Default)</option>
        <option value="#ff8c42">Orange</option>
        <option value="#66d9ef">Cyan</option>
        <option value="#2ecc71">Green</option>
        <option value="#e74c3c">Red</option>
        <option value="#e91e63">Pink</option>
        <option value="custom">Custom</option>
      </select>
      <input type="color" id="customColor" value="#a855f7" style="margin-top:9px;display:none;">
    </div>

    <button class="btn-settings" style="width:100%;margin-top:14px;" onclick="openMotorCtrl()">Manual Motor Control</button>
    <button class="btn-save" onclick="saveSettings()">Save Settings</button>
    <button class="btn-close" onclick="closeSettings()">Close</button>
  </div>
</div>

<!-- ===== MOTOR PANEL ===== -->
<div id="motorPanel" class="settings-panel">
  <div class="settings-content">
    <h3>Manual Motor Control</h3>
    <div class="lock-ind" id="lockInd">Locked during animations</div>
    <div class="set-section">
      <div id="motorSliders">
        <div class="mslider"><label><span>S0 R1</span><span id="m1v">90&deg;</span></label><input type="range" id="motor1" min="0" max="180" value="90" oninput="updateMotor(1,this.value)"></div>
        <div class="mslider"><label><span>S1 R2</span><span id="m2v">90&deg;</span></label><input type="range" id="motor2" min="0" max="180" value="90" oninput="updateMotor(2,this.value)"></div>
        <div class="mslider"><label><span>S2 L1</span><span id="m3v">90&deg;</span></label><input type="range" id="motor3" min="0" max="180" value="90" oninput="updateMotor(3,this.value)"></div>
        <div class="mslider"><label><span>S3 L2</span><span id="m4v">90&deg;</span></label><input type="range" id="motor4" min="0" max="180" value="90" oninput="updateMotor(4,this.value)"></div>
        <div class="mslider"><label><span>S4 R4</span><span id="m5v">90&deg;</span></label><input type="range" id="motor5" min="0" max="180" value="90" oninput="updateMotor(5,this.value)"></div>
        <div class="mslider"><label><span>S5 R3</span><span id="m6v">90&deg;</span></label><input type="range" id="motor6" min="0" max="180" value="90" oninput="updateMotor(6,this.value)"></div>
        <div class="mslider"><label><span>S6 L3</span><span id="m7v">90&deg;</span></label><input type="range" id="motor7" min="0" max="180" value="90" oninput="updateMotor(7,this.value)"></div>
        <div class="mslider"><label><span>S7 L4</span><span id="m8v">90&deg;</span></label><input type="range" id="motor8" min="0" max="180" value="90" oninput="updateMotor(8,this.value)"></div>
      </div>
    </div>
    <button class="btn-close" onclick="closeMotorCtrl()">Close</button>
  </div>
</div>

<script>
// ===================================================
// COMMAND QUEUE
// ===================================================
let cmdQ = 0;
const MAX_Q = 3;
let mLocked = false;

function updQ() {
  const el = document.getElementById('qStatus');
  el.textContent = 'Command Queue: ' + cmdQ + '/' + MAX_Q;
  el.classList.toggle('full', cmdQ >= MAX_Q);
}
function canSend() { return cmdQ < MAX_Q; }
function incQ() {
  cmdQ++;
  updQ();
  setTimeout(() => { if (cmdQ > 0) cmdQ--; updQ(); }, 4000); // Fallback timeout
}
function lockMotors(d = 4000) {
  mLocked = true;
  document.getElementById('lockInd').classList.add('on');
  for (let i = 1; i <= 8; i++) { const s = document.getElementById('motor' + i); if (s) s.disabled = true; }
  setTimeout(() => {
    mLocked = false;
    document.getElementById('lockInd').classList.remove('on');
    for (let i = 1; i <= 8; i++) { const s = document.getElementById('motor' + i); if (s) s.disabled = false; }
  }, d);
}

// ===================================================
// MOVEMENT (with queue — for poses)
// ===================================================
function move(dir) { if (!canSend()) return; incQ(); fetch('/cmd?go=' + dir).catch(()=>{}); }
function stop()    { cmdQ = 0; updQ(); fetch('/cmd?stop=1').catch(()=>{}); }
function pose(n)   { if (!canSend()) return; incQ(); lockMotors(3000); fetch('/cmd?pose=' + n).catch(()=>{}); }
function updateMotor(n, v) {
  if (mLocked) return;
  document.getElementById('m' + n + 'v').textContent = v + '\u00B0';
  if (!canSend()) return; incQ();
  fetch('/cmd?motor=' + n + '&value=' + v).catch(()=>{});
}

// Direct movement — bypasses queue (for joystick/tilt)
function jmove(dir) { fetch('/cmd?go=' + dir).catch(()=>{}); }
function jstop()    { fetch('/cmd?stop=1').catch(()=>{}); }

// Face command
function setFaceCmd(name) { fetch('/setFace?face=' + name).catch(()=>{}); }

// ===================================================
// CONTROL MODE
// ===================================================
let ctrlMode = localStorage.getItem('ctrlMode') || 'joystick';
let joystickInst = null;
let tiltActive = false;
let lastTiltDir = '';
let lastTiltMs = 0;

function setCtrlMode(mode) {
  ctrlMode = mode;
  localStorage.setItem('ctrlMode', mode);

  document.getElementById('dpadWrap').style.display      = (mode === 'dpad')     ? 'block' : 'none';
  document.getElementById('joystickWrap').style.display  = (mode === 'joystick') ? 'flex'  : 'none';
  document.getElementById('tiltWrap').style.display      = (mode === 'tilt')     ? 'flex'  : 'none';

  ['dpad','joystick','tilt'].forEach(m => {
    document.getElementById('pill-' + m).classList.toggle('on', m === mode);
  });
  const sel = document.getElementById('ctrlModeSelect');
  if (sel) sel.value = mode;

  if (mode === 'joystick' && !joystickInst) initJoystick();
  if (mode === 'tilt')     startTilt();
  else                     stopTilt();
}

function initCtrlMode() {
  setCtrlMode(ctrlMode);
  if (ctrlMode === 'joystick') initJoystick();
}

// ===================================================
// VIRTUAL JOYSTICK
// ===================================================
function initJoystick() {
  const canvas = document.getElementById('joystickCanvas');
  if (!canvas) return;
  joystickInst = new VirtualJoystick(canvas);
}

class VirtualJoystick {
  constructor(canvas) {
    this.c    = canvas;
    this.ctx  = canvas.getContext('2d');
    this.w    = canvas.width;
    this.h    = canvas.height;
    this.cx   = this.w / 2;
    this.cy   = this.h / 2;
    this.R    = this.w * 0.38;   // base radius
    this.kr   = this.w * 0.175;  // knob radius
    this.kx   = this.cx;
    this.ky   = this.cy;
    this.down = false;
    this.tid  = null;
    this.lastDir = '';
    this.draw();
    this.bind();
  }

  bind() {
    const c = this.c;
    c.addEventListener('touchstart',  e => { e.preventDefault(); const t = e.changedTouches[0]; this.tid = t.identifier; this.down = true; this.upd(t.clientX, t.clientY); }, {passive:false});
    c.addEventListener('touchmove',   e => { e.preventDefault(); for (const t of e.changedTouches) if (t.identifier === this.tid) this.upd(t.clientX, t.clientY); }, {passive:false});
    c.addEventListener('touchend',    e => { this.release(); });
    c.addEventListener('touchcancel', e => { this.release(); });
    c.addEventListener('mousedown', e => { this.down = true; this.updMouse(e); });
    document.addEventListener('mousemove', e => { if (this.down) this.updMouse(e); });
    document.addEventListener('mouseup',   e => { if (this.down) this.release(); });
  }

  upd(cx, cy) {
    const rect = this.c.getBoundingClientRect();
    const sx = this.w / rect.width;
    const sy = this.h / rect.height;
    this.kx = (cx - rect.left) * sx;
    this.ky = (cy - rect.top)  * sy;
    this.clamp();
    this.process();
    this.draw();
  }

  updMouse(e) { this.upd(e.clientX, e.clientY); }

  clamp() {
    const dx = this.kx - this.cx, dy = this.ky - this.cy;
    const d  = Math.hypot(dx, dy);
    if (d > this.R) { this.kx = this.cx + dx/d * this.R; this.ky = this.cy + dy/d * this.R; }
  }

  process() {
    const dx = this.kx - this.cx, dy = this.ky - this.cy;
    const d  = Math.hypot(dx, dy);
    const dz = this.R * 0.28;
    if (d < dz) { if (this.lastDir) { jstop(); this.lastDir = ''; } return; }
    const dir = (Math.abs(dx) > Math.abs(dy))
      ? (dx > 0 ? 'right' : 'left')
      : (dy > 0 ? 'backward' : 'forward');
    if (dir !== this.lastDir) { jmove(dir); this.lastDir = dir; }
  }

  release() {
    this.down = false; this.tid = null;
    this.kx = this.cx; this.ky = this.cy;
    this.draw();
    if (this.lastDir) { jstop(); this.lastDir = ''; }
  }

  draw() {
    const ctx = this.ctx, cx = this.cx, cy = this.cy, R = this.R, kr = this.kr;
    ctx.clearRect(0, 0, this.w, this.h);

    // Outer ring glow
    const glowG = ctx.createRadialGradient(cx,cy,R*0.7,cx,cy,R+6);
    glowG.addColorStop(0,'transparent');
    glowG.addColorStop(1,'rgba(168,85,247,0.08)');
    ctx.beginPath(); ctx.arc(cx,cy,R+6,0,Math.PI*2);
    ctx.fillStyle = glowG; ctx.fill();

    // Base circle
    ctx.beginPath(); ctx.arc(cx,cy,R,0,Math.PI*2);
    ctx.fillStyle = 'rgba(56,189,248,0.05)'; ctx.fill();
    ctx.strokeStyle = 'rgba(56,189,248,0.28)'; ctx.lineWidth = 1.5; ctx.stroke();

    // Crosshairs
    ctx.strokeStyle = 'rgba(56,189,248,0.12)'; ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(cx,cy-R); ctx.lineTo(cx,cy+R); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(cx-R,cy); ctx.lineTo(cx+R,cy); ctx.stroke();

    // Inner dead-zone indicator
    ctx.beginPath(); ctx.arc(cx,cy,R*0.28,0,Math.PI*2);
    ctx.strokeStyle='rgba(168,85,247,0.15)'; ctx.lineWidth=1; ctx.stroke();

    // Knob shadow
    ctx.beginPath(); ctx.arc(this.kx+2, this.ky+3, kr, 0, Math.PI*2);
    ctx.fillStyle = 'rgba(0,0,0,0.35)'; ctx.fill();

    // Knob gradient
    const kg = ctx.createRadialGradient(this.kx-kr*0.3,this.ky-kr*0.3,0,this.kx,this.ky,kr);
    kg.addColorStop(0,'rgba(196,132,252,0.95)');
    kg.addColorStop(0.6,'rgba(168,85,247,0.85)');
    kg.addColorStop(1,'rgba(56,189,248,0.75)');
    ctx.beginPath(); ctx.arc(this.kx,this.ky,kr,0,Math.PI*2);
    ctx.fillStyle = kg; ctx.fill();
    ctx.strokeStyle = 'rgba(255,255,255,0.22)'; ctx.lineWidth = 1.5; ctx.stroke();

    // Knob inner highlight
    const hl = ctx.createRadialGradient(this.kx-kr*0.3,this.ky-kr*0.35,0,this.kx,this.ky,kr*0.6);
    hl.addColorStop(0,'rgba(255,255,255,0.25)');
    hl.addColorStop(1,'rgba(255,255,255,0)');
    ctx.beginPath(); ctx.arc(this.kx,this.ky,kr,0,Math.PI*2);
    ctx.fillStyle = hl; ctx.fill();
  }
}

// ===================================================
// TILT MODE (DeviceOrientation)
// ===================================================
let tiltWatchdogTimer = null;

function tiltSetStatus(txt, color) {
  const el = document.getElementById('tiltStatus');
  if (el) { el.textContent = txt; if (color) el.style.color = color; }
}
function tiltSetErr(txt) {
  const el = document.getElementById('tiltErr');
  if (el) { el.textContent = txt; el.style.display = txt ? 'block' : 'none'; }
}

function startTilt() {
  tiltActive = true;
  lastTiltDir = '';
  tiltSetStatus('Sens\u00f6r ba\u011flan\u0131yor...');
  tiltSetErr('');
  document.getElementById('tiltDbg').textContent = '';
  ['forward','backward','left','right'].forEach(d => {
    const el = document.getElementById('ta-' + d);
    if (el) el.classList.remove('t-active');
  });

  if (typeof DeviceOrientationEvent !== 'undefined' &&
      typeof DeviceOrientationEvent.requestPermission === 'function') {
    // iOS 13+ — permission required
    document.getElementById('tiltPermBtn').style.display = 'inline-block';
    tiltSetStatus('iOS izni gerekiyor');
  } else if (typeof DeviceOrientationEvent === 'undefined') {
    tiltSetStatus('Desteklenmiyor', '#f87171');
    tiltSetErr('Bu taray\u0131c\u0131 e\u011fim sens\u00f6r\u00fcn\u00fc desteklemiyor.');
  } else {
    // Android / non-iOS — try absolute first, fallback to relative
    window.addEventListener('deviceorientationabsolute', handleTilt, true);
    window.addEventListener('deviceorientation', handleTilt, true);
    // Watchdog: if no event in 3s, warn user
    tiltWatchdogTimer = setTimeout(() => {
      if (!tiltActive) return;
      tiltSetStatus('Sens\u00f6r al\u0131nam\u0131yor!', '#f87171');
      tiltSetErr('\u26a0\ufe0f Taray\u0131c\u0131 sens\u00f6r\u00fc engelledi. Chrome\'da HTTPS gerekebilir. Firefox veya di\u011fer bir taray\u0131c\u0131 deneyin.');
    }, 3000);
  }
}

function stopTilt() {
  tiltActive = false;
  if (tiltWatchdogTimer) { clearTimeout(tiltWatchdogTimer); tiltWatchdogTimer = null; }
  window.removeEventListener('deviceorientationabsolute', handleTilt, true);
  window.removeEventListener('deviceorientation', handleTilt, true);
  if (lastTiltDir) { jstop(); lastTiltDir = ''; }
  document.getElementById('tiltPermBtn').style.display = 'none';
  ['forward','backward','left','right'].forEach(d => {
    const el = document.getElementById('ta-' + d);
    if (el) el.classList.remove('t-active');
  });
}

function reqTiltPerm() {
  DeviceOrientationEvent.requestPermission().then(s => {
    if (s === 'granted') {
      document.getElementById('tiltPermBtn').style.display = 'none';
      window.addEventListener('deviceorientation', handleTilt, true);
      tiltWatchdogTimer = setTimeout(() => {
        if (!tiltActive) return;
        tiltSetStatus('Sens\u00f6r al\u0131nam\u0131yor!', '#f87171');
        tiltSetErr('\u26a0\ufe0f iOS\'ta sens\u00f6r verisi gelmiyor. Ayarlar > Safari > Hareket ve Y\u00f6n\'\u00fc kontrol edin.');
      }, 3000);
    } else {
      tiltSetErr('\u26a0\ufe0f \u0130zin reddedildi. Ayarlar\'dan taray\u0131c\u0131 izinlerini kontrol edin.');
    }
  }).catch(e => { tiltSetErr('\u26a0\ufe0f \u0130zin istenemedi: ' + e.message); });
}

let tiltFirstEvent = false;
let tiltSubMode = 'drive'; // 'drive' or 'avatar'

function setTiltMode(mode) {
  tiltSubMode = mode;
  document.getElementById('pill-tilt-drive').classList.toggle('on', mode === 'drive');
  document.getElementById('pill-tilt-avatar').classList.toggle('on', mode === 'avatar');
}

function handleTilt(e) {
  if (!tiltActive) return;
  const now = Date.now();

  // First event received — clear watchdog & update status
  if (!tiltFirstEvent) {
    tiltFirstEvent = true;
    if (tiltWatchdogTimer) { clearTimeout(tiltWatchdogTimer); tiltWatchdogTimer = null; }
    tiltSetErr('');
  }

  if (now - lastTiltMs < 80) return;
  lastTiltMs = now;

  const beta = e.beta, gamma = e.gamma;
  if (beta === null || gamma === null) return;

  // Show raw sensor values for diagnostics
  const dbg = document.getElementById('tiltDbg');
  if (dbg) dbg.textContent = '\u03b2:' + beta.toFixed(1) + '\u00b0  \u03b3:' + gamma.toFixed(1) + '\u00b0';

  if (tiltSubMode === 'avatar') {
    // Avatar mode: Send raw angles
    // Limit and invert signs as needed. 
    // beta (pitch): positive is forward. gamma (roll): positive is right.
    const pitch = Math.max(-45, Math.min(45, Math.round(beta)));
    const roll = Math.max(-45, Math.min(45, Math.round(gamma)));
    fetch('/cmd?avatarPitch=' + pitch + '&avatarRoll=' + roll).catch(()=>{});
    
    ['forward','backward','left','right'].forEach(d => {
      const el = document.getElementById('ta-' + d);
      if (el) el.classList.remove('t-active');
    });
    return;
  }

  const thresh = 14;
  let dir = '';
  if (Math.abs(gamma) > Math.abs(beta)) {
    if      (gamma < -thresh) dir = 'left';
    else if (gamma >  thresh) dir = 'right';
  } else {
    if      (beta  < -thresh) dir = 'forward';
    else if (beta  >  thresh) dir = 'backward';
  }

  // Update arrow indicators every tick
  ['forward','backward','left','right'].forEach(d => {
    const el = document.getElementById('ta-' + d);
    if (el) el.classList.toggle('t-active', d === dir);
  });

  if (dir !== lastTiltDir) {
    dir ? jmove(dir) : jstop();
    lastTiltDir = dir;
    const dirLabel = {forward:'\u2191 \u0130leri', backward:'\u2193 Geri', left:'\u2190 Sol', right:'\u2192 Sa\u011f'};
    tiltSetStatus(dir ? dirLabel[dir] : 'N\u00f6tr \u2014 d\u00fcz tut', dir ? 'var(--accent2)' : 'var(--text-secondary)');
  }
}
// Reset first-event flag when tilt mode starts
const _origStartTilt = startTilt;
startTilt = function() { tiltFirstEvent = false; _origStartTilt(); };

// ===================================================
// THEME
// ===================================================
function loadTheme() {
  const saved = localStorage.getItem('themeColor');
  if (saved && saved !== 'purple') applyTheme(saved);
}
function applyTheme(color) {
  if (color === 'purple' || !color) return;
  const root = document.documentElement;
  root.style.setProperty('--accent', color);
  const rgb = hexRgb(color);
  if (rgb) {
    root.style.setProperty('--accent-glow', 'rgba('+rgb.r+','+rgb.g+','+rgb.b+',0.3)');
    root.style.setProperty('--glass-border','rgba('+rgb.r+','+rgb.g+','+rgb.b+',0.22)');
  }
}
function hexRgb(hex) {
  const r = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return r ? {r:parseInt(r[1],16),g:parseInt(r[2],16),b:parseInt(r[3],16)} : null;
}

// ===================================================
// BATTERY & STATUS POLLING
// ===================================================
function pollBattery() {
  fetch('/api/status').then(r=>r.json()).then(d => {
    setBatteryUI(d.batteryPercent);
    setHungerUI(d.hungerPercent);
    setLoveUI(d.lovePercent);
    if (d.currentCommand === "") {
      cmdQ = 0; updQ();
    }
  }).catch(()=>{});
}
function setBatteryUI(pct) {
  const fill = document.getElementById('batFill');
  const txt  = document.getElementById('batPct');
  if (!fill || pct < 0) return;
  const clamped = Math.max(0, Math.min(100, pct));
  fill.style.width = clamped + '%';
  txt.textContent  = clamped + '%';
  if      (clamped > 60) fill.style.background = 'linear-gradient(90deg,#22c55e,#4ade80)';
  else if (clamped > 25) fill.style.background = 'linear-gradient(90deg,#f59e0b,#fbbf24)';
  else                   fill.style.background = 'linear-gradient(90deg,#ef4444,#f87171)';
}

function setHungerUI(pct) {
  const fill = document.getElementById('hunFill');
  const txt  = document.getElementById('hunPct');
  if (!fill || pct < 0) return;
  const clamped = Math.max(0, Math.min(100, pct));
  fill.style.width = clamped + '%';
  txt.textContent  = clamped + '%';
  if      (clamped > 60) fill.style.background = 'linear-gradient(90deg,#eab308,#facc15)';
  else if (clamped > 25) fill.style.background = 'linear-gradient(90deg,#f97316,#fb923c)';
  else                   fill.style.background = 'linear-gradient(90deg,#ef4444,#f87171)';
}

function setLoveUI(pct) {
  const fill = document.getElementById('loveFill');
  const txt  = document.getElementById('lovePct');
  if (!fill || pct < 0) return;
  const clamped = Math.max(0, Math.min(100, pct));
  fill.style.width = clamped + '%';
  txt.textContent  = clamped + '%';
  if      (clamped > 60) fill.style.background = 'linear-gradient(90deg,#ec4899,#f472b6)';
  else if (clamped > 25) fill.style.background = 'linear-gradient(90deg,#d946ef,#e879f9)';
  else                   fill.style.background = 'linear-gradient(90deg,#9333ea,#c084fc)';
}

// ===================================================
// TOUCH SENSOR POLLING
// ===================================================
function pollTouch() {
  fetch('/getTouchStatus').then(r=>r.json()).then(data => {
    const touched = data.touched === true || data.touched === "true";
    const d1 = document.getElementById('tDotMain');
    const d2 = document.getElementById('tDotSettings');
    const ts = document.getElementById('tStatusTxt');
    if (d1) d1.classList.toggle('on', touched);
    if (d2) d2.classList.toggle('on', touched);
    if (ts) ts.textContent = touched ? 'Miu sevildi! \uD83D\uDC96' : 'Miu\'nun kafa sens\u00f6r\u00fc: bekliyor';
    if (touched) {
      const sys = d1 && d1.closest('.section');
      if (sys && !sys.classList.contains('tpulse')) {
        sys.classList.add('tpulse');
        setTimeout(()=>sys.classList.remove('tpulse'), 600);
      }
    }
  }).catch(()=>{});
}

// ===================================================
// SETTINGS
// ===================================================
function openSettings() {
  fetch('/getSettings').then(r=>r.json()).then(d => {
    document.getElementById('frameDelay').value       = d.frameDelay || 100;
    document.getElementById('walkCycles').value       = d.walkCycles || 10;
    document.getElementById('motorCurrentDelay').value= d.motorCurrentDelay || 20;
    if (document.getElementById('autonomousMode')) {
      document.getElementById('autonomousMode').checked = (d.autonomousMode === true);
    }
    const savedColor = localStorage.getItem('themeColor') || 'purple';
    const sel = document.getElementById('themeColor');
    let found = false;
    for (const o of sel.options) { if (o.value === savedColor) { sel.value = savedColor; found = true; break; } }
    if (!found) { sel.value='custom'; document.getElementById('customColor').value=savedColor; document.getElementById('customColor').style.display='block'; }
  }).catch(()=>{
    document.getElementById('frameDelay').value = 100;
    document.getElementById('walkCycles').value = 10;
    document.getElementById('motorCurrentDelay').value = 20;
  });
  document.getElementById('ctrlModeSelect').value = ctrlMode;
  document.getElementById('settingsPanel').style.display = 'block';

  document.getElementById('themeColor').addEventListener('change', function() {
    if (this.value==='custom') document.getElementById('customColor').style.display='block';
    else { document.getElementById('customColor').style.display='none'; applyTheme(this.value); }
  });
  document.getElementById('customColor').addEventListener('input', function() { applyTheme(this.value); });
}
function closeSettings() { document.getElementById('settingsPanel').style.display='none'; }

function openMotorCtrl() { document.getElementById('motorPanel').style.display='block'; }
function closeMotorCtrl() { document.getElementById('motorPanel').style.display='none'; }

function saveSettings() {
  const fd  = document.getElementById('frameDelay').value;
  const wc  = document.getElementById('walkCycles').value;
  const mcd = document.getElementById('motorCurrentDelay').value;
  const ms  = document.getElementById('motorSpeed') ? document.getElementById('motorSpeed').value : 'medium';
  const cm  = document.getElementById('ctrlModeSelect').value;
  const am  = document.getElementById('autonomousMode') ? document.getElementById('autonomousMode').checked : false;

  const colorSel = document.getElementById('themeColor');
  const customCol= document.getElementById('customColor');
  const themeColor = colorSel.value==='custom' ? customCol.value : colorSel.value;
  localStorage.setItem('themeColor', themeColor);
  localStorage.setItem('ctrlMode', cm);
  applyTheme(themeColor);
  setCtrlMode(cm);

  fetch('/setSettings?frameDelay='+fd+'&walkCycles='+wc+'&motorCurrentDelay='+mcd+'&autonomousMode='+am)
    .then(()=>closeSettings()).catch(()=>closeSettings());
}

// ===================================================
// GAMEPAD
// ===================================================
let activeGP = null, gpPollId = null;
let lastBtns = [], lastAxis = {x:0,y:0};
const AX_THR = 0.5, GP_POLL = 80;

const gpBindings = {
  0:()=>pose('stand'),1:()=>pose('wave'),2:()=>pose('dance'),3:()=>pose('swim'),
  4:()=>pose('point'),5:()=>pose('pushup'),6:()=>pose('bow'),7:()=>pose('shake'),
  8:()=>stop(),9:()=>pose('rest'),10:()=>pose('cute'),11:()=>pose('freaky'),
  12:()=>move('forward'),13:()=>move('backward'),14:()=>move('left'),15:()=>move('right'),
  16:()=>stop(),17:()=>pose('worm')
};
const gpRelStop = new Set([12,13,14,15]);

function gpStatus(on) {
  const el = document.getElementById('gamepadStatus');
  if (!el) return;
  el.textContent = on ? 'Gamepad connected' : 'Gamepad disconnected';
  el.classList.toggle('connected', on);
}
function gpBtn(i, pressed) {
  if (pressed) { const a = gpBindings[i]; if (a) a(); }
  else if (gpRelStop.has(i)) stop();
}
function gpAxisDir(x,y) {
  if (Math.abs(x)<AX_THR&&Math.abs(y)<AX_THR) return {x:0,y:0};
  return Math.abs(x)>Math.abs(y) ? {x:x>0?1:-1,y:0} : {x:0,y:y>0?1:-1};
}
function applyAxis(d) {
  if(d.x===1)move('right'); else if(d.x===-1)move('left');
  else if(d.y===1)move('backward'); else if(d.y===-1)move('forward');
  else stop();
}
function pollGP() {
  const pads=navigator.getGamepads?navigator.getGamepads():[];
  const pad=pads&&activeGP!==null?pads[activeGP]:null;
  if(!pad){gpStatus(false);return;} gpStatus(true);
  if(!lastBtns.length) lastBtns=pad.buttons.map(b=>!!b.pressed);
  pad.buttons.forEach((b,i)=>{ const p=!!b.pressed; if(p!==lastBtns[i]){gpBtn(i,p);lastBtns[i]=p;} });
  const d=gpAxisDir(pad.axes[0]||0,pad.axes[1]||0);
  if(d.x!==lastAxis.x||d.y!==lastAxis.y){applyAxis(d);lastAxis=d;}
}
window.addEventListener('gamepadconnected',e=>{activeGP=e.gamepad.index;lastBtns=[];lastAxis={x:0,y:0};gpStatus(true);if(!gpPollId)gpPollId=setInterval(pollGP,GP_POLL);});
window.addEventListener('gamepaddisconnected',e=>{if(activeGP===e.gamepad.index){activeGP=null;lastBtns=[];lastAxis={x:0,y:0};gpStatus(false);}});
if(navigator.getGamepads){setInterval(()=>{if(activeGP!==null)return;const pads=navigator.getGamepads();if(!pads)return;for(let i=0;i<pads.length;i++){if(pads[i]){activeGP=pads[i].index;gpStatus(true);if(!gpPollId)gpPollId=setInterval(pollGP,GP_POLL);break;}}},1000);}

// ===================================================
// PET INTERACTIONS
// ===================================================
function petAction(url, btn, successText) {
  const orig = btn.innerHTML;
  btn.disabled = true;
  btn.style.opacity = '0.6';
  fetch(url).then(r => {
    if (r.ok) { btn.innerHTML = '&#10004; ' + successText; }
    else { btn.innerHTML = '&#10060; Hata!'; }
    setTimeout(() => { btn.innerHTML = orig; btn.disabled = false; btn.style.opacity = '1'; }, 1500);
  }).catch(() => {
    btn.innerHTML = '&#10060; Ba\u011Flant\u0131 Yok';
    setTimeout(() => { btn.innerHTML = orig; btn.disabled = false; btn.style.opacity = '1'; }, 1500);
  });
}
function feedMiu(e)   { petAction('/api/feed',   e ? e.target : this, 'Afiyetle Yedi!'); }
function tickleMiu(e) { petAction('/api/tickle', e ? e.target : this, 'Gıdıklandı!'); }
function waterMiu(e)  { petAction('/api/water',  e ? e.target : this, 'Lıkır Lıkır İçti!'); }
function findMiu(e)   { petAction('/api/find',   e ? e.target : this, 'Buradayım!'); }

function playJukebox(song, btn) { petAction('/api/jukebox?song='+song, btn, 'Oynat\u0131l\u0131yor!'); }
function sendEmoji(mood) { fetch('/api/emoji?mood='+mood).catch(()=>{}); }

// ===================================================
// SPEECH & MELODY
// ===================================================
function sendSpeech() {
  const text = document.getElementById('speechTxt').value;
  if(!text) return;
  fetch('/speech?text=' + encodeURIComponent(text)).catch(()=>{});
  document.getElementById('speechTxt').value = '';
}

let melF=[], melD=[];
function addNote(f,d) {
  if(melF.length>=32) return;
  melF.push(f); melD.push(d);
  document.getElementById('melStr').textContent = melF.length + " nota eklendi";
}
function clearMelody() {
  melF=[]; melD=[];
  document.getElementById('melStr').textContent = "Temizlendi";
}
function playMelody() {
  if(melF.length===0) return;
  const url = '/customMelody?freqs='+melF.join(',')+'&durs='+melD.join(',');
  fetch(url).catch(()=>{});
  melF=[]; melD=[];
  document.getElementById('melStr').textContent = "\u00c7al\u0131n\u0131yor...";
}

// ===================================================
// PIXEL ART CUSTOM FACE
// ===================================================
let pixelTool = 'pen';
let isDrawing = false;
let pixelCtx = null;

function initPixelCanvas() {
  const pCanvas = document.getElementById('pixelCanvas');
  if (!pCanvas) return;
  pixelCtx = pCanvas.getContext('2d', { willReadFrequently: true });
  // Fill background with black
  pixelCtx.fillStyle = '#000';
  pixelCtx.fillRect(0, 0, 128, 64);

  pCanvas.addEventListener('mousedown', pStart);
  window.addEventListener('mousemove', pDraw);
  window.addEventListener('mouseup', pStop);
  
  pCanvas.addEventListener('touchstart', pStart, {passive:false});
  pCanvas.addEventListener('touchmove', pDraw, {passive:false});
  window.addEventListener('touchend', pStop);
}

function getPos(e) {
  const pCanvas = document.getElementById('pixelCanvas');
  const rect = pCanvas.getBoundingClientRect();
  const clientX = e.touches ? e.touches[0].clientX : e.clientX;
  const clientY = e.touches ? e.touches[0].clientY : e.clientY;
  const scaleX = pCanvas.width / rect.width;
  const scaleY = pCanvas.height / rect.height;
  return {
    x: Math.floor((clientX - rect.left) * scaleX),
    y: Math.floor((clientY - rect.top) * scaleY)
  };
}

function pStart(e) {
  if(e.target.id !== 'pixelCanvas') return;
  e.preventDefault();
  isDrawing = true;
  pDraw(e);
}

function pDraw(e) {
  if (!isDrawing) return;
  if (e.cancelable) e.preventDefault();
  const pos = getPos(e);
  if (pos.x < 0 || pos.x >= 128 || pos.y < 0 || pos.y >= 64) return;
  
  pixelCtx.fillStyle = (pixelTool === 'pen') ? '#fff' : '#000';
  // Brush size 2x2 for easier drawing on mobile
  pixelCtx.fillRect(pos.x, pos.y, 2, 2);
}

function pStop() { isDrawing = false; }

function setPixelTool(tool) {
  pixelTool = tool;
  document.getElementById('btnPen').style.background = (tool === 'pen') ? 'rgba(168,85,247,.25)' : '';
  document.getElementById('btnEraser').style.background = (tool === 'eraser') ? 'rgba(168,85,247,.25)' : '';
}

function clearPixelCanvas() {
  if(!pixelCtx) return;
  pixelCtx.fillStyle = '#000';
  pixelCtx.fillRect(0, 0, 128, 64);
}

function sendCustomFace(event) {
  if(!pixelCtx) return;
  const imgData = pixelCtx.getImageData(0, 0, 128, 64).data;
  let hexString = '';
  
  for (let page = 0; page < 8; page++) {
    for (let col = 0; col < 128; col++) {
      let byte = 0;
      for (let bit = 0; bit < 8; bit++) {
        let y = page * 8 + bit;
        let x = col;
        let index = (y * 128 + x) * 4;
        let r = imgData[index];
        if (r > 128) { // If pixel is white
          byte |= (1 << bit);
        }
      }
      hexString += byte.toString(16).padStart(2, '0');
    }
  }
  
  const btn = event.target;
  const origText = btn.innerHTML;
  btn.innerHTML = 'G&ouml;nderiliyor...';
  
  fetch('/api/customFace', {
    method: 'POST',
    body: hexString
  }).then(r => {
    if(r.ok) btn.innerHTML = '&#10004; G&ouml;nderildi!';
    else btn.innerHTML = '&#10060; Hata!';
    setTimeout(() => { btn.innerHTML = origText; }, 2000);
  }).catch(()=>{
    btn.innerHTML = '&#10060; Ba\u011Flant\u0131 Hatas\u0131';
    setTimeout(() => { btn.innerHTML = origText; }, 2000);
  });
}

// ===================================================
// INIT
// ===================================================
document.addEventListener('DOMContentLoaded', () => {
  loadTheme();
  initCtrlMode();
  initPixelCanvas();
  pollBattery();
  setInterval(pollBattery, 3000);
  setInterval(pollTouch, 400);
});
</script>
</body>
</html>
)rawliteral";

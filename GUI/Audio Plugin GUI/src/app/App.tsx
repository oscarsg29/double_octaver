import { useState } from "react";
import { RotaryKnob } from "./components/RotaryKnob";
import { OctaveSelector } from "./components/OctaveSelector";

const ACCENT = "#e8730a";
const BLUE = "#4a9eff";

export default function App() {
  const [dryGain, setDryGain] = useState(0.7);
  const [masterGain, setMasterGain] = useState(0.6);
  const [oct1Gain, setOct1Gain] = useState(0.5);
  const [oct2Gain, setOct2Gain] = useState(0.35);
  const [oct1Position, setOct1Position] = useState(1);
  const [oct2Position, setOct2Position] = useState(2);
  const [power, setPower] = useState(true);

  return (
    <div
      style={{
        minHeight: "100vh",
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        background: "#0f0f12",
        fontFamily: "'Rajdhani', sans-serif",
      }}
    >
      <div
        style={{
          width: "460px",
          background: "linear-gradient(165deg, #252528 0%, #1a1a1e 60%, #141418 100%)",
          borderRadius: "8px",
          border: "1px solid rgba(255,255,255,0.09)",
          boxShadow:
            "0 4px 6px rgba(0,0,0,0.5), 0 20px 60px rgba(0,0,0,0.8), inset 0 1px 0 rgba(255,255,255,0.06)",
          overflow: "hidden",
          userSelect: "none",
        }}
      >
        {/* Header */}
        <div
          style={{
            background: "linear-gradient(90deg, #111114 0%, #1c1c21 50%, #111114 100%)",
            borderBottom: "1px solid rgba(255,255,255,0.06)",
            padding: "7px 14px",
            display: "flex",
            alignItems: "center",
            justifyContent: "space-between",
          }}
        >
          <div style={{ display: "flex", alignItems: "baseline", gap: "8px" }}>
            <span
              style={{
                fontFamily: "'Rajdhani', sans-serif",
                fontSize: "18px",
                fontWeight: 700,
                color: ACCENT,
                letterSpacing: "0.18em",
                textTransform: "uppercase",
              }}
            >
              OCTAVER
            </span>
            <span
              style={{
                fontFamily: "'Share Tech Mono', monospace",
                fontSize: "9px",
                color: "rgba(232,228,216,0.3)",
                letterSpacing: "0.1em",
              }}
            >
              v1.1
            </span>
          </div>

          <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
            <div
              style={{
                width: "6px",
                height: "6px",
                borderRadius: "50%",
                background: power ? ACCENT : "#333",
                boxShadow: power ? `0 0 6px ${ACCENT}, 0 0 12px ${ACCENT}66` : "none",
                transition: "all 0.2s",
              }}
            />
            <button
              onClick={() => setPower(!power)}
              style={{
                fontFamily: "'Share Tech Mono', monospace",
                fontSize: "8px",
                letterSpacing: "0.14em",
                textTransform: "uppercase",
                color: power ? ACCENT : "rgba(232,228,216,0.25)",
                background: "none",
                border: `1px solid ${power ? ACCENT + "44" : "rgba(255,255,255,0.06)"}`,
                borderRadius: "3px",
                padding: "2px 7px",
                cursor: "pointer",
                transition: "all 0.15s",
                outline: "none",
              }}
            >
              {power ? "ON" : "OFF"}
            </button>
          </div>
        </div>

        {/* Screw row */}
        <div
          style={{
            display: "flex",
            justifyContent: "space-between",
            padding: "5px 12px 0",
            opacity: 0.35,
          }}
        >
          {[0, 1, 2, 3].map((i) => <Screw key={i} />)}
        </div>

        {/* Main controls */}
        <div
          style={{
            padding: "12px 18px 16px",
            display: "flex",
            gap: "0",
            opacity: power ? 1 : 0.35,
            transition: "opacity 0.3s",
            pointerEvents: power ? "auto" : "none",
            alignItems: "stretch",
          }}
        >
          {/* Octave voices section */}
          <div style={{ flex: 1, paddingRight: "18px" }}>
            <SectionLabel>Octave Voices</SectionLabel>
            <div style={{ display: "flex", gap: "16px", justifyContent: "flex-start" }}>
              {/* Voice 1 */}
              <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "8px" }}>
                <RotaryKnob
                  label="Oct 1"
                  value={oct1Gain}
                  onChange={setOct1Gain}
                  color={ACCENT}
                  size={58}
                />
                <OctaveSelector
                  label="Voice 1"
                  value={oct1Position}
                  onChange={setOct1Position}
                  color={ACCENT}
                />
              </div>

              {/* Voice 2 */}
              <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: "8px" }}>
                <RotaryKnob
                  label="Oct 2"
                  value={oct2Gain}
                  onChange={setOct2Gain}
                  color={BLUE}
                  size={58}
                />
                <OctaveSelector
                  label="Voice 2"
                  value={oct2Position}
                  onChange={setOct2Position}
                  color={BLUE}
                />
              </div>
            </div>
          </div>

          {/* Vertical divider */}
          <div
            style={{
              background:
                "linear-gradient(180deg, transparent, rgba(255,255,255,0.06) 20%, rgba(255,255,255,0.06) 80%, transparent)",
              width: "1px",
              flexShrink: 0,
            }}
          />

          {/* Output section */}
          <div style={{ paddingLeft: "18px", display: "flex", flexDirection: "column" }}>
            <SectionLabel>Output</SectionLabel>
            <div style={{ display: "flex", flexDirection: "column", gap: "14px", alignItems: "center", flex: 1, justifyContent: "center" }}>
              <RotaryKnob
                label="Dry"
                value={dryGain}
                onChange={setDryGain}
                color="#e8e4d8"
                size={58}
              />
              <RotaryKnob
                label="Master"
                value={masterGain}
                onChange={setMasterGain}
                color="#a0c878"
                size={58}
              />
            </div>
          </div>
        </div>

        {/* Status strip */}
        <div
          style={{
            borderTop: "1px solid rgba(255,255,255,0.05)",
            background: "#111114",
            padding: "5px 14px",
            display: "flex",
            alignItems: "center",
            justifyContent: "space-between",
          }}
        >
          <span
            style={{
              fontFamily: "'Share Tech Mono', monospace",
              fontSize: "8px",
              color: "rgba(232,228,216,0.18)",
              letterSpacing: "0.1em",
            }}
          >
            PITCH SHIFT ENGINE
          </span>
          <div style={{ display: "flex", gap: "12px" }}>
            <StatusDot label="DRY" active color="#e8e4d8" />
            <StatusDot label="V1" active={power} color={ACCENT} />
            <StatusDot label="V2" active={power} color={BLUE} />
            <StatusDot label="OUT" active={power} color="#a0c878" />
          </div>
        </div>
      </div>
    </div>
  );
}

function SectionLabel({ children }: { children: React.ReactNode }) {
  return (
    <div
      style={{
        fontFamily: "'Share Tech Mono', monospace",
        fontSize: "8px",
        color: "rgba(232,228,216,0.22)",
        letterSpacing: "0.2em",
        textTransform: "uppercase",
        borderBottom: "1px solid rgba(255,255,255,0.04)",
        paddingBottom: "6px",
        marginBottom: "10px",
      }}
    >
      {children}
    </div>
  );
}

function Screw() {
  return (
    <svg width="8" height="8" viewBox="0 0 10 10">
      <circle cx="5" cy="5" r="4.5" fill="#1a1a1e" stroke="rgba(255,255,255,0.15)" strokeWidth="0.5" />
      <line x1="2" y1="5" x2="8" y2="5" stroke="rgba(255,255,255,0.2)" strokeWidth="0.8" />
      <line x1="5" y1="2" x2="5" y2="8" stroke="rgba(255,255,255,0.2)" strokeWidth="0.8" />
    </svg>
  );
}

function StatusDot({ label, active, color }: { label: string; active: boolean; color: string }) {
  return (
    <div style={{ display: "flex", alignItems: "center", gap: "4px" }}>
      <div
        style={{
          width: "4px",
          height: "4px",
          borderRadius: "50%",
          background: active ? color : "#333",
          boxShadow: active ? `0 0 4px ${color}` : "none",
          transition: "all 0.2s",
        }}
      />
      <span
        style={{
          fontFamily: "'Share Tech Mono', monospace",
          fontSize: "7px",
          color: active ? color + "99" : "rgba(255,255,255,0.12)",
          letterSpacing: "0.08em",
          transition: "color 0.2s",
        }}
      >
        {label}
      </span>
    </div>
  );
}
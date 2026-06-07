interface OctaveSelectorProps {
  label: string;
  value: number; // 0..3 → -2oct, -1oct, +1oct, +2oct
  onChange: (v: number) => void;
  color?: string;
}

const POSITIONS = [
  { label: "-2", sublabel: "oct" },
  { label: "-1", sublabel: "oct" },
  { label: "+1", sublabel: "oct" },
  { label: "+2", sublabel: "oct" },
];

export function OctaveSelector({
  label,
  value,
  onChange,
  color = "#e8730a",
}: OctaveSelectorProps) {
  return (
    <div className="flex flex-col items-center gap-2">
      <span
        style={{
          fontFamily: "'Share Tech Mono', monospace",
          fontSize: "11px",
          color: "var(--muted-foreground)",
          letterSpacing: "0.14em",
          textTransform: "uppercase",
        }}
      >
        {label}
      </span>

      {/* 4-position stepped switch housing */}
      <div
        style={{
          background: "linear-gradient(180deg, #111116 0%, #1c1c22 100%)",
          borderRadius: "6px",
          border: "1px solid rgba(255,255,255,0.07)",
          boxShadow:
            "inset 0 2px 6px rgba(0,0,0,0.6), 0 1px 0 rgba(255,255,255,0.05)",
          padding: "3px",
          display: "flex",
          flexDirection: "column",
          gap: "2px",
          position: "relative",
        }}
      >
        {POSITIONS.map((pos, i) => {
          const active = value === i;
          return (
            <button
              key={i}
              onClick={() => onChange(i)}
              style={{
                width: "48px",
                height: "24px",
                borderRadius: "4px",
                border: active
                  ? `1px solid ${color}88`
                  : "1px solid rgba(255,255,255,0.04)",
                background: active
                  ? `linear-gradient(180deg, ${color}33 0%, ${color}18 100%)`
                  : "linear-gradient(180deg, #2a2a32 0%, #1e1e26 100%)",
                boxShadow: active
                  ? `0 0 8px ${color}44, inset 0 1px 0 ${color}22`
                  : "inset 0 2px 4px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.03)",
                cursor: "pointer",
                display: "flex",
                alignItems: "center",
                justifyContent: "center",
                gap: "2px",
                transition: "all 0.1s ease",
                outline: "none",
              }}
            >
              <span
                style={{
                  fontFamily: "'Share Tech Mono', monospace",
                  fontSize: "13px",
                  fontWeight: active ? 600 : 400,
                  color: active ? color : "rgba(232,228,216,0.4)",
                  lineHeight: 1,
                  letterSpacing: "0.02em",
                  transition: "color 0.1s",
                }}
              >
                {pos.label}
              </span>
              <span
                style={{
                  fontFamily: "'Share Tech Mono', monospace",
                  fontSize: "8px",
                  color: active ? `${color}bb` : "rgba(232,228,216,0.25)",
                  lineHeight: 1,
                  marginTop: "2px",
                  transition: "color 0.1s",
                }}
              >
                {pos.sublabel}
              </span>
            </button>
          );
        })}

        {/* Position indicator groove */}
        <div
          style={{
            position: "absolute",
            left: "-5px",
            top: `${3 + value * 26 + 9}px`,
            width: "3px",
            height: "8px",
            borderRadius: "2px",
            background: color,
            boxShadow: `0 0 6px ${color}`,
            transition: "top 0.12s cubic-bezier(0.4,0,0.2,1)",
          }}
        />
      </div>

      {/* Active value readout */}
      <span
        style={{
          fontFamily: "'Share Tech Mono', monospace",
          fontSize: "10px",
          color: color,
          opacity: 0.75,
          letterSpacing: "0.06em",
        }}
      >
        {POSITIONS[value].label} {POSITIONS[value].sublabel}
      </span>
    </div>
  );
}
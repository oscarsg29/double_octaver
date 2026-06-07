import { useRef, useCallback, useState } from "react";

interface RotaryKnobProps {
  label: string;
  value: number; // 0..1
  onChange: (v: number) => void;
  color?: string;
  size?: number;
  sublabel?: string;
}

const MIN_ANGLE = -145;
const MAX_ANGLE = 145;

export function RotaryKnob({
  label,
  value,
  onChange,
  color = "#e8730a",
  size = 72,
  sublabel,
}: RotaryKnobProps) {
  const angle = MIN_ANGLE + value * (MAX_ANGLE - MIN_ANGLE);
  const dragStart = useRef<{ y: number; value: number } | null>(null);
  const [dragging, setDragging] = useState(false);

  const onMouseDown = useCallback(
    (e: React.MouseEvent) => {
      e.preventDefault();
      dragStart.current = { y: e.clientY, value };
      setDragging(true);

      const onMove = (me: MouseEvent) => {
        if (!dragStart.current) return;
        const delta = (dragStart.current.y - me.clientY) / 200;
        const next = Math.min(1, Math.max(0, dragStart.current.value + delta));
        onChange(next);
      };
      const onUp = () => {
        dragStart.current = null;
        setDragging(false);
        window.removeEventListener("mousemove", onMove);
        window.removeEventListener("mouseup", onUp);
      };
      window.addEventListener("mousemove", onMove);
      window.addEventListener("mouseup", onUp);
    },
    [value, onChange]
  );

  const cx = size / 2;
  const cy = size / 2;
  const r = size / 2 - 4;
  const indicatorLen = r - 6;

  // Indicator tip position
  const rad = ((angle - 90) * Math.PI) / 180;
  const x2 = cx + Math.cos(rad) * indicatorLen;
  const y2 = cy + Math.sin(rad) * indicatorLen;

  // Arc path for value fill
  const arcStartAngle = MIN_ANGLE - 90;
  const arcEndAngle = angle - 90;
  const arcR = r - 2;
  const toRad = (deg: number) => (deg * Math.PI) / 180;
  const arcX1 = cx + Math.cos(toRad(arcStartAngle)) * arcR;
  const arcY1 = cy + Math.sin(toRad(arcStartAngle)) * arcR;
  const arcX2 = cx + Math.cos(toRad(arcEndAngle)) * arcR;
  const arcY2 = cy + Math.sin(toRad(arcEndAngle)) * arcR;
  const largeArc = angle - MIN_ANGLE > 180 ? 1 : 0;

  const displayDb = Math.round((value * 24 - 12) * 10) / 10;

  return (
    <div className="flex flex-col items-center gap-2 select-none">
      <div
        style={{
          width: size,
          height: size,
          cursor: dragging ? "grabbing" : "grab",
          filter: dragging ? `drop-shadow(0 0 8px ${color}88)` : undefined,
          transition: "filter 0.15s",
        }}
        onMouseDown={onMouseDown}
      >
        <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`}>
          {/* Outer ring */}
          <circle
            cx={cx}
            cy={cy}
            r={r}
            fill="url(#knobGrad)"
            stroke="rgba(255,255,255,0.06)"
            strokeWidth="1"
          />
          {/* Track background arc */}
          <path
            d={`M ${cx + Math.cos(toRad(arcStartAngle)) * arcR} ${cy + Math.sin(toRad(arcStartAngle)) * arcR} A ${arcR} ${arcR} 0 1 1 ${cx + Math.cos(toRad(90 - 90)) * arcR} ${cy + Math.sin(toRad(90 - 90)) * arcR}`}
            fill="none"
            stroke="rgba(255,255,255,0.07)"
            strokeWidth="3"
            strokeLinecap="round"
          />
          {/* Value arc */}
          {value > 0 && (
            <path
              d={`M ${arcX1} ${arcY1} A ${arcR} ${arcR} 0 ${largeArc} 1 ${arcX2} ${arcY2}`}
              fill="none"
              stroke={color}
              strokeWidth="3"
              strokeLinecap="round"
              opacity="0.85"
            />
          )}
          {/* Inner body */}
          <circle
            cx={cx}
            cy={cy}
            r={r - 8}
            fill="url(#knobInner)"
            stroke="rgba(0,0,0,0.5)"
            strokeWidth="1"
          />
          {/* Indicator line */}
          <line
            x1={cx}
            y1={cy}
            x2={x2}
            y2={y2}
            stroke={color}
            strokeWidth="2.5"
            strokeLinecap="round"
          />
          {/* Center dot */}
          <circle cx={cx} cy={cy} r="2.5" fill={color} opacity="0.5" />
          <defs>
            <radialGradient id="knobGrad" cx="40%" cy="35%" r="65%">
              <stop offset="0%" stopColor="#3c3c46" />
              <stop offset="100%" stopColor="#1a1a22" />
            </radialGradient>
            <radialGradient id="knobInner" cx="35%" cy="30%" r="70%">
              <stop offset="0%" stopColor="#383840" />
              <stop offset="100%" stopColor="#18181e" />
            </radialGradient>
          </defs>
        </svg>
      </div>
      <div
        className="flex flex-col items-center"
        style={{ fontFamily: "'Share Tech Mono', monospace", gap: "1px" }}
      >
        <span
          style={{
            fontSize: "9px",
            color: "var(--muted-foreground)",
            letterSpacing: "0.12em",
            textTransform: "uppercase",
          }}
        >
          {label}
        </span>
        <span
          style={{
            fontSize: "10px",
            color: color,
            letterSpacing: "0.04em",
          }}
        >
          {displayDb > 0 ? "+" : ""}
          {displayDb} dB
        </span>
      </div>
    </div>
  );
}
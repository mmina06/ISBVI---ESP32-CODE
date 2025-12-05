
const int DOT[6]    = {33, 32, 13, 12, 14, 27}; // Dot1..Dot6
const int SPACE_PIN = 4;                         // Space (INPUT_PULLUP)

const uint16_t CHORD_MS    = 250;
const uint16_t DEBOUNCE_MS = 20;
const uint16_t AFTER_MS    = 60;

bool numericMode = false;

#define B(d) (1u << ((d)-1))

// ---------- Standard UEB letters ----------
struct Pair { uint8_t mask; char ch; };
const Pair LETTERS[] = {
  {B(1)                ,'a'}, {B(1)|B(2)           ,'b'}, {B(1)|B(4)           ,'c'},
  {B(1)|B(4)|B(5)      ,'d'}, {B(1)|B(5)           ,'e'}, {B(1)|B(2)|B(4)      ,'f'},
  {B(1)|B(2)|B(4)|B(5) ,'g'}, {B(1)|B(2)|B(5)      ,'h'}, {B(2)|B(4)           ,'i'},
  {B(2)|B(4)|B(5)      ,'j'}, {B(1)|B(3)           ,'k'}, {B(1)|B(2)|B(3)      ,'l'},
  {B(1)|B(3)|B(4)      ,'m'}, {B(1)|B(3)|B(4)|B(5) ,'n'}, {B(1)|B(3)|B(5)      ,'o'},
  {B(1)|B(2)|B(3)|B(4) ,'p'}, {B(1)|B(2)|B(3)|B(4)|B(5),'q'},
  {B(1)|B(2)|B(3)|B(5) ,'r'}, {B(2)|B(3)|B(4)      ,'s'}, {B(2)|B(3)|B(4)|B(5) ,'t'},
  {B(1)|B(3)|B(6)      ,'u'}, {B(1)|B(2)|B(3)|B(6) ,'v'}, {B(2)|B(4)|B(5)|B(6) ,'w'},
  {B(1)|B(3)|B(4)|B(6) ,'x'}, {B(1)|B(3)|B(4)|B(5)|B(6),'y'}, {B(1)|B(3)|B(5)|B(6) ,'z'}
};

// Numbers / symbols
const uint8_t NUMBER_SIGN = B(3)|B(4)|B(5)|B(6); // 3456
const uint8_t BS_MASK     = B(2)|B(5)|B(6);      // Backspace = dots 2-5-6

struct Sym { uint8_t mask; char out; };
const Sym SYMBOLS[] = {
  { B(3)|B(6)        , '-' }, { B(3)|B(4)        , '/' },
  { B(3)|B(4)|B(6)   , '+' }, { B(3)|B(5)|B(6)   , '*' },
  { B(2)|B(3)|B(5)|B(6), '=' }, { B(1)|B(2)|B(6) , '(' },
  { B(4)|B(5)|B(6)   , ')' }, { B(2)|B(5)        , ',' }
};

inline bool isPressed(int pin){ return digitalRead(pin) == LOW; }
bool anyDotPressed(){ for (int i=0;i<6;i++) if (isPressed(DOT[i])) return true; return false; }

void waitReleaseAll(){
  unsigned long t0 = millis();
  while (anyDotPressed() || isPressed(SPACE_PIN)) {
    if (millis() - t0 > 800) break;
    delay(5);
  }
  delay(AFTER_MS);
}

char mapLetter(uint8_t m){ for (auto &p : LETTERS) if (p.mask == m) return p.ch; return '\0'; }
char mapSymbol(uint8_t m){ for (auto &s : SYMBOLS) if (s.mask == m) return s.out; return '\0'; }

char aToJ_toDigit(char c){
  switch(c){
    case 'a': return '1'; case 'b': return '2'; case 'c': return '3';
    case 'd': return '4'; case 'e': return '5'; case 'f': return '6';
    case 'g': return '7'; case 'h': return '8'; case 'i': return '9';
    case 'j': return '0'; default:  return '\0';
  }
}

void setup(){
  Serial.begin(115200);
  for (int i=0;i<6;i++) pinMode(DOT[i], INPUT_PULLUP);
  pinMode(SPACE_PIN, INPUT_PULLUP);

  Serial.println("✅ Braille Typewriter READY!");
  Serial.println("Dots = {33,32,13,12,14,27} | Space = 4 | Backspace = dots 2-5-6");
  Serial.println("3456 = Number Mode | Dot2 in Number Mode = '.'");
}

void loop(){
  // Space (tap) -> print space and exit numeric mode
  if (isPressed(SPACE_PIN) && !anyDotPressed()){
    delay(DEBOUNCE_MS);
    if (isPressed(SPACE_PIN) && !anyDotPressed()){
      Serial.write(' ');
      numericMode = false;
      waitReleaseAll();
      return;
    }
  }

  if (!anyDotPressed()){ delay(2); return; }

  // Capture chord
  uint8_t mask = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < CHORD_MS){
    for (int i=0;i<6;i++){
      if (isPressed(DOT[i])) {
        mask |= B(i+1);   // FIXED HERE 🔥🔥🔥
      }
    }
    delay(2);
  }

  // Backspace (dots 2-5-6)
  if (mask == BS_MASK){
    Serial.write('\b'); Serial.write(' '); Serial.write('\b');
    waitReleaseAll();
    return;
  }

  // Number sign → enter number mode
  if (mask == NUMBER_SIGN){
    numericMode = true;
    waitReleaseAll();
    return;
  }

  // Decimal point in number mode
  if (numericMode && mask == B(2)){
    Serial.write('.');
    waitReleaseAll();
    return;
  }

  // Symbols
  if (char sym = mapSymbol(mask)){
    Serial.write(sym);
    waitReleaseAll();
    return;
  }

  // Letters / digits
  if (char letter = mapLetter(mask)){
    if (numericMode){
      if (char d = aToJ_toDigit(letter)) Serial.write(d);
      else { Serial.write(letter); numericMode = false; }
    } else {
      Serial.write(letter);
    }
    waitReleaseAll();
    return;
  }

  waitReleaseAll();
}


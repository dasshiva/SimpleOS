from enum import Enum
from fixedint import *
import copy, sys

class Scanner:
  def __init__(self, file):
    self.file = file
    self.handle = open(file, "r")
    self.line = 1
    self.text = ""
  
  def change_comma(self):
    new = ""
    for i in self.text:
      if i == ',':
        continue
      new += i
    self.text = new
    
  def next(self):
    self.text = self.handle.readline()
    if self.text == "":
      return ["EOF"]
    elif self.text[0] == '/' and self.text[1] == '/':
      return ["BLANK"]
    self.change_comma()
    return self.text.split()
    
  def error(self, msg):
    print(f"In {self.file} at line {self.line} : {self.text}\nError: {msg}")
    import sys
    sys.exit(1)

sc = Scanner(sys.argv[1])

insn_mapper = [
  ['define', 3, 1, 'TIV'],
  ['add', 2, 2, 'II'],
  ['addi', 2, 3, 'TI'],
  ['sub', 2, 4, 'II'],
  ['subi', 2, 5, 'TI'],
  ['mul', 2, 6, 'II'],
  ['muli', 2, 7, 'TI'],
  ['div', 2, 8, 'II'],
  ['divi', 2, 9, 'TI']
]

TYPES = [
  'u8',
  'u16',
  'u32',
  'u64'
]

class Token(Enum):
  TYPE = 1,
  IMM = 2,
  ID = 3,
  LABEL = 4,
  INSN = 5

class Parser:
  def __init__(self, scan):
    self.src = scan
    self.tokens = []
    self.symtab = []
  
  def tokenise(self, text):
    for token in text:
      if token[0].isnumeric():
        try:
          self.tokens.append((Token.IMM, int(token)))
        except:
          self.src.error(f"Malformed number {token}")

      elif token[-1] == ':':
        self.tokens.append(( Token.LABEL, token ))

      elif token[0] == '"':
        self.tokens.append((Token.STRING, token[1:len(token)-1]))
      else:
        for ty in TYPES:
          if ty == token:
            self.tokens.append((Token.TYPE, ty))
            return 
        self.tokens.append(( Token.INSN, token ))
  
  def parse_with(self, form, parsed):
    parse = []
    for i, c in enumerate(form, 1):
      if c == 'T' and self.tokens[i][0] != Token.TYPE:
        parse.clear()
        return False
      elif c == 'I' and self.tokens[i][0] != Token.INSN:
        parse.clear()
        return False
      elif c == 'V' and self.tokens[i][0] != Token.IMM:
        parse.clear()
        return False
      elif c == 'N':
        break
      else:
        parse.append(self.tokens[i][1])
    parsed += parse
    parsed.append(form)
    return True
  
  def parse_insn(self):
    parsed = []
    index = -1

    for c, cont in enumerate(insn_mapper):
      if (self.tokens[0][1] == cont[0]):
        index = c
        break
    
    if index == -1:
      self.src.error(f"Invalid instruction {self.tokens[0][1]}")
    
    ins = insn_mapper[index]
    parsed.append(ins[2])
    if self.parse_with(ins[3], parsed):
      return parsed
    self.src.error(f"Instruction {self.tokens[0][1]} has been given invalid or incorrect number of arguments")
  
  def parse(self):
    while True:
      line = sc.next()
      if line[0] == 'BLANK':
        continue 
      if line[0] != "EOF":
        self.tokenise(line)
        for tok in self.tokens:
          if tok[0] == Token.INSN:
            parsed = self.parse_insn()
            if len(self.symtab) == 0:
              self.src.error("Instructions are not allowed at top level")
            self.symtab[-1].append(parsed)
            self.tokens.clear()
            break
          elif tok[0] == Token.LABEL:
            self.symtab.append([tok[1]])
            self.tokens.clear()
            break
          else:
            self.src.error("Only labels are allowed at top level")
      else:
        break;
      print(self.symtab)

  def codegen(self):
    pass  

sc = Scanner(sys.argv[1])
p = Parser(sc)
p.parse()
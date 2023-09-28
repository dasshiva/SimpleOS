import sys

class Compiler:
    content = []
    def __init__(self, file):
        self.file = open(file, 'r')
        for line in self.file:
            

import re

with open(r'help\build_your_own.TXT', 'r', encoding='utf-8') as f:
    text = f.read()

text = text.replace('# Build Your Own BASIC++ Dialect: The Definitive Guide', '# Build Your Own BASIC++ Environment: The Definitive Guide')
text = text.replace('build a dialect that consumes as little as 40KB', 'build an environment that consumes as little as 40KB')
text = text.replace('Bypasses the loading of the standard dialect suite.', 'Bypasses the loading of standard libraries.')

text = re.sub(r'### Legacy Dialect Compatibility \(Saves ~200KB\)\n- \*\*Omit Directory:\*\* source/dialect/ \(Except dialect\.c\)\n- \*Production Note:\* BASIC\+\+ supports runtime switching \(dialect_gwbs\.c, dialect_qbasic\.c, dialect_ecma116\.c\)\. In a custom build, rely on the default strict parser and omit these files to save ROM space\.\n', '', text)

text = text.replace('source/dialect/dialect_gwbs.c source/dialect/dialect_qbasic.c', '')

with open(r'help\build_your_own.TXT', 'w', encoding='utf-8') as f:
    f.write(text)

with open(r'help\porting_guide.TXT', 'r', encoding='utf-8') as f:
    text2 = f.read()
    
text2 = text2.replace('memory footprint and bypassing dialect loading.', 'memory footprint.')

with open(r'help\porting_guide.TXT', 'w', encoding='utf-8') as f:
    f.write(text2)


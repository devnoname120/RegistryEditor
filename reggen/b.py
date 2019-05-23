

numtotext = {}

def parse_bas(line):
    parts = line.split('=')
    numtotext[parts[0]]=parts[1]

def output_key(path, k, kt, ks):
    outpath = "/" + ''.join(path)

    print('\t{"%s", "%s", %d, %d },' % (outpath, k, kt, ks))

def parse_reg(line):
#    print(line)
    path, data = line.split('=')

#    print(path)
    pathparts = path.split('/')
#    print(pathparts)
    vparts = data.split(':')
    keytype = int(vparts[0])
    keysize = int(vparts[1])

    key = numtotext[pathparts[-1]]

    pparts = []
    for p in pathparts[1:-1]:
        pparts.append(numtotext[p])

#    print(pparts)

    rang = False
    for p in range(0, len(pparts)):
        if "-" in pparts[p]:
#            print("rang = ", p, pparts[p])
            rang = p
            break

    if not rang:
#        print(pparts)
        output_key(pparts, key, keytype, keysize)
    else:
        rfrom, rto = pparts[rang].split('-')
        rfrom = int(rfrom)
        rto = int(rto[:-1])
        for i in range(rfrom, rto+1):
            pparts[rang] = "%02x/" % i
            output_key(pparts, key, keytype, keysize)

part = ""
with open("registry.dec", "rt") as fin:
    while True:
        line = fin.readline()
        if len(line) == 0:
            break

        line = line.strip()

        if line[0] == '[':
            part = line[1:]
            continue

        if part == "BASE":
            parse_bas(line)
        elif part == "REG-BAS":
            parse_reg(line)
        else:
            break

#print(numtotext)

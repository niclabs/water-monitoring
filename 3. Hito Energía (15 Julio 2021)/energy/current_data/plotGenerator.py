import argparse
import pandas as pd
import matplotlib.pyplot as plt

def initParser():
    parser = argparse.ArgumentParser(description='File info')
    parser.add_argument('file_name', type=str, help='File name')
    parser.add_argument('--plot_name', dest='new_file', type=str, help='Name of the file to store plot')
    return parser

def negativeToZero(x):
    if x < 0:
        return 0
    return x

def main():
    img_path = 'img/'
    parser = initParser()
    args = parser.parse_args()
    df = pd.read_csv(args.file_name, sep=',', engine='python', index_col=None, header=None, skiprows=30, skipfooter=30)
    first_time = df[1][0]
    df[1] = df[1].map(lambda x: x - first_time)
    df[1] = df[1].map(lambda x: x/1000000)
    df[0] = df[0].map(negativeToZero)
    fig, ax = plt.subplots()
    ax.plot(df[1], df[0], linewidth=0.3)
    ax.set_xlabel('Segundos')
    ax.set_ylabel('mA')
    plt.savefig(img_path+args.file_name.split('.')[0]+'.png')
    plt.show()

if __name__=="__main__":
    main()

            


import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set_theme(style='whitegrid')
sns.set_context('talk', font_scale=1.2)
plt.rcParams.update({
    'figure.titlesize': 18,
    'axes.titlesize': 16,
    'axes.labelsize': 14,
    'xtick.labelsize': 12,
    'ytick.labelsize': 12,
    'legend.fontsize': 12,
    'legend.title_fontsize': 13,
})

translate = {
    'alfaMax': 'alfa máximo',
    'pageSize': 'tamanho de página',
    'P_alfaMax': 'combinações'
}

def load_and_prepare_data(filepath):
    """
    Loads the CSV data and calculates the necessary percentage columns.
    Also creates a combined column for the (pageSize, alfaMax) pair
    and calculates disk accesses per query.
    """
    df = pd.read_csv(filepath, sep=';')
    
    # Fix potential typo in CSV header
    if 'oveflow' in df.columns:
        df.rename(columns={'oveflow': 'overflow'}, inplace=True)
        
    # Calculate percentages
    df['overflow_pct'] = (df['overflow'] / (df['overflow'] + df['capacity'])) * 100
    df['alfa_distance_pct'] = (abs(df['alfaMax'] - df['alfa']) / df['alfaMax']) * 100
    
    # Calculate disk accesses per query
    df['accessPerQueryIn'] = df['diskAccessIn'] / df['searchIn']
    df['accessPerQueryOut'] = df['diskAccessOut'] / df['searchOut']
    
    # Create the combined pair column
    df['P_alfaMax'] = df.apply(lambda row: f"({int(row['pageSize'])}, {row['alfaMax']})", axis=1)
    
    return df

def plot_disk_access(df, x_axis, save_dir=None, suf=""):
    """
    Generates a side-by-side boxplot of disk accesses per query (In vs Out).
    """
    plt.figure(figsize=(10, 6))
    melted_df = df.melt(id_vars=[x_axis], 
                        value_vars=['accessPerQueryIn', 'accessPerQueryOut'], 
                        var_name='Tipo de busca', 
                        value_name='Accesses Per Query')
    
    # Rename the Tipo de buscas for a cleaner legend
    melted_df['Tipo de busca'] = melted_df['Tipo de busca'].replace({
        'accessPerQueryIn': 'presentes', 
        'accessPerQueryOut': 'ausentes'
    })
    
    sns.boxplot(data=melted_df, x=x_axis, y='Accesses Per Query', hue='Tipo de busca', palette='Set2')
    plt.title(f'Acesso médio ao disco por busca (presentes vs ausentes) por {translate[x_axis]}')
    plt.xlabel(translate[x_axis].capitalize())
    plt.ylabel('Acesso médio ao disco por busca')
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    if save_dir:
        os.makedirs(save_dir, exist_ok=True)
        file_path = os.path.join(save_dir, f'disk_access_per_query_by_{x_axis}{f"_{suf}" if suf!="" else suf}.png')
        plt.savefig(file_path, bbox_inches='tight')
        print(f"Saved: {file_path}")
        plt.close()
    else:
        plt.show()

def plot_overflow_percentage(df, x_axis, save_dir=None, suf=""):
    """
    Generates a boxplot of the percentage of overflow relative to capacity.
    """
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=df, x=x_axis, y='overflow_pct', color='lightcoral')
    plt.title(
        f'Páginas de Overflow em relação ao total de páginas '
        f'por {translate[x_axis]}'
    )
    plt.xlabel(translate[x_axis].capitalize())
    plt.ylabel('Porcentagem de Overflow (%)')
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    if save_dir:
        os.makedirs(save_dir, exist_ok=True)
        file_path = os.path.join(save_dir, f'overflow_percentage_by_{x_axis}{f"_{suf}" if suf!="" else suf}.png')
        plt.savefig(file_path, bbox_inches='tight')
        print(f"Saved: {file_path}")
        plt.close()
    else:
        plt.show()

def plot_alfa_distance(df, x_axis, save_dir=None, suf=""):
    """
    Generates a boxplot of the percentual distance between alfaMax and alfa.
    Only renders if the x_axis includes 'pageSize' (P).
    """
        
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=df, x=x_axis, y='alfa_distance_pct', color='skyblue')
    plt.title(f'Distância percentual do valor de alfa para o valor máximo de alfa')
    plt.xlabel(translate[x_axis])
    plt.ylabel('Distância para alfa máximo (%)')
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    if save_dir:
        os.makedirs(save_dir, exist_ok=True)
        file_path = os.path.join(save_dir, f'alfa_distance_by_{x_axis}{f"_{suf}" if suf!="" else suf}.png')
        plt.savefig(file_path, bbox_inches='tight')
        print(f"Saved: {file_path}")
        plt.close()
    else:
        plt.show()

def generate_all_graphs(filepath, x_axis='pageSize', save_dir=None, filter_alfa=None, filter_pageSize=None, suf=""):
    """
    Main function to load data and generate all applicable graphs based on the chosen x_axis.
    Options for x_axis: 'pageSize', 'alfaMax', or 'P_alfaMax'
    Optional filters can be passed as lists, e.g., filter_alfa=[0.60, 0.75]
    """
    if x_axis not in ['pageSize', 'alfaMax', 'P_alfaMax']:
        raise ValueError("x_axis must be 'pageSize', 'alfaMax', or 'P_alfaMax'")
        
    print(f"\n--- Generating graphs using '{x_axis}' as the X-axis ---")
    
    # Load data
    df = load_and_prepare_data(filepath)
    
    # Apply filters if provided
    if filter_alfa is not None:
        df = df[df['alfaMax'].isin(filter_alfa)]
        print(f"Filtered data to include only alfa values: {filter_alfa}")
        
    if filter_pageSize is not None:
        df = df[df['pageSize'].isin(filter_pageSize)]
        print(f"Filtered data to include only pageSize values: {filter_pageSize}")
        
    # Prevent errors if the filter removes all data
    if df.empty:
        print("Warning: The filtered dataset is empty. Check your filter values. No graphs will be generated.")
        return
    
    # Plot and save/show graphs
    plot_disk_access(df, x_axis, save_dir, suf)
    plot_overflow_percentage(df, x_axis, save_dir, suf)
    plot_alfa_distance(df, x_axis, save_dir, suf)
    print("--- Done ---")

if __name__ == "__main__":
    csv_file_path = "results/table.csv"
    output_directory = "images/"

    # Example 1: Use 'pageSize' as the X-axis, but only plot for alfa values 0.60 and 0.75
    generate_all_graphs(
        csv_file_path, 
        x_axis='pageSize', 
        save_dir=output_directory,
    )
    generate_all_graphs(
        csv_file_path, 
        x_axis='alfaMax', 
        save_dir=output_directory, 
    )

    generate_all_graphs(
        csv_file_path, 
        x_axis='pageSize', 
        save_dir=output_directory,
        filter_alfa=[0.6,0.75],
        suf="060_075"
    )
    generate_all_graphs(
        csv_file_path, 
        x_axis='pageSize', 
        save_dir=output_directory,
        filter_alfa=[0.8,0.9],
        suf="080_090"
    )
    generate_all_graphs(
        csv_file_path, 
        x_axis='alfaMax', 
        save_dir=output_directory, 
        filter_pageSize=[10,25],
        suf="10_25"
    )
    generate_all_graphs(
        csv_file_path, 
        x_axis='alfaMax', 
        save_dir=output_directory, 
        filter_pageSize=[50,100],
        suf="50_100"
    )
    
    # Não irá para o relatório final
    # Usado para fins de análise preliminar
    generate_all_graphs(
        csv_file_path, 
        x_axis='P_alfaMax', 
        save_dir=output_directory,
    )
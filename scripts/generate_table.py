import pandas as pd

def generate_latex_table(csv_filepath, output_filepath):
    """
    Reads the CSV data, calculates the metrics, aggregates them by mean,
    and outputs a formatted LaTeX table.
    """
    # 1. Load data
    df = pd.read_csv(csv_filepath, sep=';')
    
    # Fix typo if it exists
    if 'oveflow' in df.columns:
        df.rename(columns={'oveflow': 'overflow'}, inplace=True)
        
    # 2. Calculate new columns using abbreviations for a compact LaTeX table
    df['D(\\%)'] = (abs(df['alfaMax'] - df['alfa']) / df['alfaMax']) * 100
    df['$S_{In}$'] = df['diskAccessIn'] / df['searchIn']
    df['$S_{Out}$'] = df['diskAccessOut'] / df['searchOut']
    df['O(\\%)'] = (df['overflow'] / (df['capacity'] + df['overflow'])) * 100
    
    # Ensure this list is in the exact order you requested
    metrics = ['D(\\%)', '$S_{In}$', '$S_{Out}$', 'O(\\%)']
    
    # 3. Pivot and aggregate the data (using mean for synthesis)
    pivot_df = pd.pivot_table(
        df,
        index='alfaMax',
        columns='pageSize',
        values=metrics,
        aggfunc='mean'
    )
    
    # 4. Reorder MultiIndex columns
    # Swap levels so that 'pageSize' becomes the super-column
    pivot_df = pivot_df.swaplevel(0, 1, axis=1)
    
    # Sort columns by 'pageSize' to group them properly
    pivot_df = pivot_df.sort_index(axis=1, level=0)
    
    # Enforce our specific metric order inside the super-columns
    pivot_df = pivot_df.reindex(metrics, level=1, axis=1)
    
    # --- NEW: Dynamic Column Formatting ---
    # Find out how many page sizes (super columns) exist to format the vertical lines
    num_page_sizes = len(pivot_df.columns.levels[0]) # type: ignore
    
    # 'l' for the index, and '|cccc' for each group of 4 metrics. 
    # E.g., for 2 page sizes it outputs: l|cccc|cccc
    col_format = "l" + "|cccc" * num_page_sizes
    
    # 5. Generate LaTeX string
    latex_str = pivot_df.to_latex(
        float_format="%.2f",
        na_rep="-",
        multicolumn=True,
        multicolumn_format='c|', 
        caption="Synthesized Metric Averages by alfaMax and pageSize",
        label="tab:synthesized_metrics",
        column_format=col_format 
    )
    
    # --- ROBUST FIX: Centralize and Resize Table ---
    if '\\begin{table}' in latex_str:
        # Open the resizebox
        latex_str = latex_str.replace(
            '\\begin{table}\n', 
            '\\begin{table}[htbp]\n\\centering\n\\resizebox{\\textwidth}{!}{%\n'
        )
        
    # Close the resizebox right after \end{tabular}. 
    # Removed the '\n' from the search string to guarantee it finds the match!
    if '\\end{tabular}' in latex_str:
        latex_str = latex_str.replace(
            '\\end{tabular}', 
            '\\end{tabular}%\n}'
        )
    # 6. Save to file
    with open(output_filepath, 'w') as f:
        f.write(latex_str)
        
    print(f"\n--- Done ---")
    print(f"LaTeX table successfully generated and saved to: {output_filepath}")

# ==========================================
# Example Usage
# ==========================================
if __name__ == "__main__":
    csv_file_path = "results/table.csv"
    tex_output_path = "latex/table.tex"
    
    generate_latex_table(csv_file_path, tex_output_path)
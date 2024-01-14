import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Exemplo de valores normalizados do acelerômetro
x_normalized = 0.5
y_normalized = -0.3
z_normalized = 0.8

# Mapeamento para posições conhecidas
x_real = x_normalized * 10  # Assumindo que o intervalo é de -10 a 10
y_real = y_normalized * 10
z_real = z_normalized * 10

# Visualização gráfica
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(x_real, y_real, z_real, c='r', marker='o')

ax.set_xlabel('Eixo X')
ax.set_ylabel('Eixo Y')
ax.set_zlabel('Eixo Z')

plt.show()

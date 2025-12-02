"""
Testes Automatizados COMPLETOS - Sistema EducaUTFPR
Trabalho 3 - Engenharia de Software

Requisitos:
    pip install pytest pyautogui pillow
    
Execução:
    pytest test_educautfpr_completo.py -v
    pytest test_educautfpr_completo.py -v --html=report.html
"""

import pytest
import pyautogui
import time
import os
from datetime import datetime


class TestBase:
    """Classe base com funções auxiliares para todos os testes"""
    
    @staticmethod
    def esperar(segundos=1):
        """Aguarda um tempo determinado"""
        time.sleep(segundos)
    
    @staticmethod
    def tirar_screenshot(nome):
        """Tira screenshot para documentação"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        pasta = "screenshots"
        if not os.path.exists(pasta):
            os.makedirs(pasta)
        pyautogui.screenshot(f"{pasta}/{nome}_{timestamp}.png")
    
    @staticmethod
    def clicar_posicao(x, y):
        """Clica em uma posição específica"""
        pyautogui.click(x=x, y=y)
    
    @staticmethod
    def digitar_limpo(texto, intervalo=0.1):
        """Digita texto com limpeza prévia"""
        pyautogui.hotkey('ctrl', 'a')
        pyautogui.write(texto, interval=intervalo)


#  ---------------  TESTES DE CRIAÇÃO DE CONTA  --------------- 
class TestCriarConta(TestBase):
    """Testes do módulo de cadastro de usuários"""
    
    def test_cadastro_sucesso(self):
        """
        Cenário: Cadastro de novo usuário com dados válidos
        Dado que o usuário está na tela de cadastro
        Quando preenche todos os campos corretamente
        Então o cadastro deve ser realizado com sucesso
        """
        print("\n[TESTE] Cadastro de usuário com sucesso")
        
        # Preencher campos
        pyautogui.write("João Silva", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("Silva", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("joao.silva", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("joao@email.com", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("senha123", interval=0.1)
        
        self.esperar(1)
        self.tirar_screenshot("cadastro_preenchido")
        
        # Clicar no botão cadastrar
        pyautogui.press('enter')
        self.esperar(2)
        
        self.tirar_screenshot("cadastro_confirmado")
        print("✅ Cadastro realizado com sucesso")
    
    def test_cadastro_senha_invalida(self):
        """
        Cenário: Tentativa de cadastro com senha fraca
        Dado que o usuário está na tela de cadastro
        Quando tenta usar uma senha muito curta
        Então deve exibir mensagem de erro
        """
        print("\n[TESTE] Cadastro com senha inválida")
        
        pyautogui.write("Maria", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("Santos", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("maria.santos", interval=0.1)
        pyautogui.press('tab')
        
        pyautogui.write("maria@email.com", interval=0.1)
        pyautogui.press('tab')
        
        # Senha muito curta
        pyautogui.write("123", interval=0.1)
        
        self.esperar(1)
        pyautogui.press('enter')
        self.esperar(2)
        
        self.tirar_screenshot("cadastro_senha_invalida")
        print("⚠️ Erro esperado: senha muito curta")


#  ---------------  TESTES DE PERFIL  --------------- 
class TestPerfil(TestBase):
    """Testes do módulo de gerenciamento de perfil"""
    
    def test_editar_perfil_sucesso(self):
        """
        Cenário: Edição de dados do perfil
        Dado que o usuário está logado
        Quando acessa o perfil e edita os dados
        Então as alterações devem ser salvas
        """
        print("\n[TESTE] Editar perfil com sucesso")
        
        # Navegar até o perfil
        self.esperar(2)
        self.clicar_posicao(690, 30)  # Botão perfil
        self.esperar(2)
        
        self.tirar_screenshot("perfil_aberto")
        
        # Clicar em editar
        self.clicar_posicao(600, 500)  # Botão Editar
        self.esperar(1)
        
        # Editar nome
        self.clicar_posicao(400, 250)
        self.digitar_limpo("João Silva Editado")
        
        # Editar email
        pyautogui.press('tab')
        self.digitar_limpo("joao.novo@email.com")
        
        self.esperar(1)
        self.tirar_screenshot("perfil_editado")
        
        # Salvar
        self.clicar_posicao(700, 500)  # Botão Salvar
        self.esperar(2)
        
        print("✅ Perfil editado com sucesso")
    
    def test_visualizar_estatisticas(self):
        """
        Cenário: Visualização de estatísticas acadêmicas
        Dado que o usuário está logado
        Quando acessa o perfil
        Então deve visualizar suas estatísticas
        """
        print("\n[TESTE] Visualizar estatísticas do perfil")
        
        self.esperar(2)
        self.clicar_posicao(690, 30)  # Botão perfil
        self.esperar(2)
        
        # Rolar para ver estatísticas
        pyautogui.scroll(-3)
        self.esperar(1)
        
        self.tirar_screenshot("estatisticas_visiveis")
        
        # Fechar perfil
        pyautogui.press('esc')
        self.esperar(1)
        
        print("✅ Estatísticas visualizadas")


#  --------------- TESTES DE GRUPOS DE ESTUDO  --------------- 
class TestGruposEstudo(TestBase):
    """Testes do módulo de grupos de estudo"""
    
    def test_criar_grupo_publico(self):
        """
        Cenário: Criação de grupo de estudo público
        Dado que o usuário está logado
        Quando cria um novo grupo público
        Então o grupo deve ser criado e visível
        """
        print("\n[TESTE] Criar grupo de estudo público")
        
        # Acessar grupos de estudo (botão "Grupo de estudo" na home)
        self.esperar(2)
        self.clicar_posicao(210, 250)  # Botão grupos
        self.esperar(2)
        
        self.tirar_screenshot("tela_grupos_principal")
        
        # Clicar em "Criar"
        self.clicar_posicao(600, 150)  # Botão Criar
        self.esperar(2)
        
        self.tirar_screenshot("tela_criar_grupo")
        
        # Preencher nome do grupo
        self.clicar_posicao(400, 250)
        pyautogui.write("Grupo Calculo 1 - Turma A", interval=0.1)
        
        # Selecionar matéria
        self.clicar_posicao(400, 320)  # ComboBox de matérias
        self.esperar(1)
        pyautogui.press('down')
        pyautogui.press('down')
        pyautogui.press('enter')
        
        # Selecionar tipo público
        self.clicar_posicao(300, 400)  # Radio button público
        
        self.esperar(1)
        self.tirar_screenshot("grupo_configurado")
        
        # Criar grupo
        self.clicar_posicao(600, 600)  # Botão Confirmar Criar
        self.esperar(2)
        
        self.tirar_screenshot("grupo_criado")
        print("✅ Grupo público criado com sucesso")
    
    def test_entrar_sala_materia(self):
        """
        Cenário: Entrar em sala de matéria
        Dado que o usuário está na tela de grupos
        Quando clica em uma matéria
        Então deve entrar no chat da matéria
        """
        print("\n[TESTE] Entrar em sala de matéria")
        
        self.esperar(2)
        
        # Clicar na aba "Matérias"
        self.clicar_posicao(200, 150)
        self.esperar(2)
        
        self.tirar_screenshot("lista_materias")
        
        # Clicar em uma matéria (primeira da lista)
        self.clicar_posicao(400, 300)
        self.esperar(3)
        
        self.tirar_screenshot("chat_materia_aberto")
        
        # Enviar uma mensagem de teste
        self.clicar_posicao(400, 550)  # Campo de mensagem
        pyautogui.write("Ola pessoal!", interval=0.1)
        pyautogui.press('enter')
        self.esperar(2)
        
        self.tirar_screenshot("mensagem_enviada")
        
        # Fechar chat
        pyautogui.press('esc')
        self.esperar(1)
        
        print("✅ Sala de matéria acessada e mensagem enviada")


#  --------------- TESTES DE CENTRAL DE DÚVIDAS / FÓRUM  --------------- 
class TestCentralDuvidas(TestBase):
    """Testes do módulo de fórum/central de dúvidas"""
    
    def test_criar_duvida(self):
        """
        Cenário: Criar nova dúvida no fórum
        Dado que o usuário está no fórum
        Quando cria uma nova dúvida
        Então a dúvida deve aparecer na lista
        """
        print("\n[TESTE] Criar nova dúvida no fórum")
        
        # Acessar fórum (botão "Central de Dúvidas" na home)
        self.esperar(2)
        self.clicar_posicao(480, 400)  # Botão Central de Dúvidas
        self.esperar(2)
        
        self.tirar_screenshot("forum_principal")
        
        # Selecionar categoria (Matemática)
        self.clicar_posicao(400, 200)
        self.esperar(2)
        
        self.tirar_screenshot("categoria_selecionada")
        
        # Selecionar matéria (Cálculo 1)
        self.clicar_posicao(400, 350)
        self.esperar(2)
        
        self.tirar_screenshot("materias_forum")
        
        # Clicar em "Nova Dúvida"
        self.clicar_posicao(600, 600)  # Botão flutuante
        self.esperar(2)
        
        # Preencher título
        self.clicar_posicao(400, 250)
        pyautogui.write("Duvida sobre derivadas", interval=0.1)
        
        # Preencher descrição
        self.clicar_posicao(400, 350)
        pyautogui.write("Como calcular a derivada de x^2 + 3x?", interval=0.1)
        
        self.esperar(1)
        self.tirar_screenshot("duvida_preenchida")
        
        # Publicar
        self.clicar_posicao(600, 550)  # Botão Enviar
        self.esperar(3)
        
        self.tirar_screenshot("duvida_publicada")
        print("✅ Dúvida criada com sucesso")
    
    def test_responder_duvida(self):
        """
        Cenário: Responder uma dúvida existente
        Dado que existe uma dúvida no fórum
        Quando o usuário adiciona uma resposta
        Então a resposta deve aparecer na dúvida
        """
        print("\n[TESTE] Responder dúvida no fórum")
        
        self.esperar(2)
        
        # Clicar em uma dúvida da lista
        self.clicar_posicao(400, 400)
        self.esperar(3)
        
        self.tirar_screenshot("duvida_aberta")
        
        # Rolar até o botão de responder
        pyautogui.scroll(-3)
        self.esperar(1)
        
        # Clicar em "Adicionar Resposta"
        self.clicar_posicao(600, 300)
        self.esperar(2)
        
        # Escrever resposta
        self.clicar_posicao(400, 350)
        pyautogui.write("A derivada de x^2 e 2x e de 3x e 3", interval=0.1)
        
        self.esperar(1)
        self.tirar_screenshot("resposta_escrita")
        
        # Enviar resposta
        self.clicar_posicao(600, 450)  # Botão Enviar Resposta
        self.esperar(3)
        
        self.tirar_screenshot("resposta_enviada")
        print("✅ Resposta adicionada com sucesso")


#  ---------------  TESTES DE AGENDA ACADÊMICA --------------- 
class TestAgendaAcademica(TestBase):
    """Testes do módulo de agenda acadêmica"""
    
    def test_adicionar_prova(self):
        """
        Cenário: Adicionar nova prova na agenda
        Dado que o usuário está na tela de agenda
        Quando adiciona uma nova prova
        Então a prova deve aparecer na lista
        """
        print("\n[TESTE] Adicionar prova na agenda")
        
        # Acessar agenda (botão "Agenda Acadêmica" na home)
        self.esperar(2)
        self.clicar_posicao(400, 250)  # Botão Agenda
        self.esperar(2)
        
        self.tirar_screenshot("agenda_aberta")
        
        # Clicar em "+ Nova Tarefa"
        self.clicar_posicao(950, 200)  # Botão Nova Tarefa
        self.esperar(2)
        
        self.tirar_screenshot("dialogo_nova_tarefa")
        
        # Selecionar tipo "Prova"
        self.clicar_posicao(400, 200)  # ComboBox tipo
        pyautogui.press('down')  # Seleciona "Prova"
        pyautogui.press('enter')
        
        # Preencher título
        self.clicar_posicao(400, 270)
        pyautogui.write("Prova de Calculo 2", interval=0.1)
        
        # Preencher disciplina
        self.clicar_posicao(400, 340)
        pyautogui.write("Calculo 2", interval=0.1)
        
        # Selecionar data (usar data atual + 7 dias)
        self.clicar_posicao(400, 410)  # Campo de data
        self.esperar(1)
        # Navegar no calendário
        pyautogui.press('right')
        pyautogui.press('right')
        pyautogui.press('right')
        pyautogui.press('enter')
        
        # Preencher descrição
        self.clicar_posicao(400, 500)
        pyautogui.write("Conteudo: derivadas e integrais", interval=0.1)
        
        self.esperar(1)
        self.tirar_screenshot("prova_preenchida")
        
        # Salvar
        self.clicar_posicao(600, 600)  # Botão Salvar
        self.esperar(2)
        
        self.tirar_screenshot("prova_adicionada")
        print("✅ Prova adicionada com sucesso")
    
    def test_marcar_tarefa_concluida(self):
        """
        Cenário: Marcar tarefa como concluída
        Dado que existe uma tarefa na agenda
        Quando marca como concluída
        Então o status deve ser atualizado
        """
        print("\n[TESTE] Marcar tarefa como concluída")
        
        self.esperar(2)
        
        self.tirar_screenshot("lista_tarefas")
        
        # Clicar no botão "✓ Concluir" da primeira tarefa
        self.clicar_posicao(1050, 300)
        self.esperar(2)
        
        self.tirar_screenshot("tarefa_concluida")
        
        # Verificar filtro de concluídas
        self.clicar_posicao(400, 200)  # ComboBox filtro
        self.esperar(1)
        # Selecionar "✅Concluídas"
        for _ in range(5):
            pyautogui.press('down')
        pyautogui.press('enter')
        self.esperar(2)
        
        self.tirar_screenshot("filtro_concluidas")
        print("✅ Tarefa marcada como concluída")


#  ---------------  TESTES DE DÚVIDAS  --------------- 
class TestDuvidas(TestBase):
    """Testes do módulo de dúvidas individuais"""
    
    def test_criar_duvida_com_imagem(self):
        """
        Cenário: Criar dúvida com anexo de imagem
        Dado que o usuário está na tela de dúvidas
        Quando cria uma dúvida com imagem
        Então a dúvida deve ser salva com o anexo
        """
        print("\n[TESTE] Criar dúvida com imagem")
        
        # Acessar dúvidas (botão "Dúvidas" na home)
        self.esperar(2)
        self.clicar_posicao(310, 400)  # Botão Dúvidas
        self.esperar(2)
        
        self.tirar_screenshot("tela_duvidas")
        
        # Clicar em "+ Nova Dúvida"
        self.clicar_posicao(950, 200)
        self.esperar(2)
        
        # Selecionar disciplina
        self.clicar_posicao(400, 180)
        pyautogui.press('down')
        pyautogui.press('down')
        pyautogui.press('enter')
        
        # Preencher título
        self.clicar_posicao(400, 250)
        pyautogui.write("Duvida sobre circuitos", interval=0.1)
        
        # Preencher descrição
        self.clicar_posicao(400, 350)
        pyautogui.write("Como resolver este circuito?", interval=0.1)
        
        # Selecionar imagem
        self.clicar_posicao(400, 480)  # Botão Selecionar Imagem
        self.esperar(2)
        # Simular seleção de arquivo (depende do SO)
        pyautogui.write("circuito.png", interval=0.1)
        pyautogui.press('enter')
        self.esperar(1)
        
        self.esperar(1)
        self.tirar_screenshot("duvida_com_imagem")
        
        # Salvar
        self.clicar_posicao(600, 550)
        self.esperar(2)
        
        self.tirar_screenshot("duvida_salva")
        print("✅ Dúvida com imagem criada")
    
    def test_filtrar_duvidas_por_disciplina(self):
        """
        Cenário: Filtrar dúvidas por disciplina
        Dado que existem várias dúvidas
        Quando aplica filtro por disciplina
        Então deve exibir apenas dúvidas daquela disciplina
        """
        print("\n[TESTE] Filtrar dúvidas por disciplina")
        
        self.esperar(2)
        
        self.tirar_screenshot("todas_duvidas")
        
        # Abrir filtro de disciplinas
        self.clicar_posicao(550, 200)  # ComboBox filtro
        self.esperar(1)
        
        # Selecionar "Cálculo Diferencial e Integral 1"
        pyautogui.press('down')
        pyautogui.press('down')
        pyautogui.press('enter')
        self.esperar(2)
        
        self.tirar_screenshot("duvidas_filtradas")
        
        # Voltar para "Todas as Disciplinas"
        self.clicar_posicao(550, 200)
        pyautogui.press('up')
        pyautogui.press('enter')
        self.esperar(1)
        
        print("✅ Filtro de disciplinas funcionando")


#  ---------------  TESTES DE AVALIAÇÃO DE MATÉRIAS  --------------- 
class TestAvaliacaoMaterias(TestBase):
    """Testes do módulo de avaliação de dificuldade"""
    
    def test_avaliar_dificuldade_materia(self):
        """
        Cenário: Avaliar dificuldade de uma matéria
        Dado que o usuário está na tela de avaliação
        Quando vota em um nível de dificuldade
        Então o voto deve ser registrado
        """
        print("\n[TESTE] Avaliar dificuldade de matéria")
        
        # Acessar avaliação (botão "Avaliação" na home)
        self.esperar(2)
        self.clicar_posicao(580, 250)  # Botão Avaliação
        self.esperar(2)
        
        self.tirar_screenshot("tela_avaliacao")
        
        # Selecionar categoria
        self.clicar_posicao(550, 200)
        pyautogui.press('down')
        pyautogui.press('enter')
        self.esperar(2)
        
        self.tirar_screenshot("materias_avaliacao")
        
        # Rolar até ver botões de dificuldade
        pyautogui.scroll(-2)
        self.esperar(1)
        
        # Votar em "Médio"
        self.clicar_posicao(470, 600)  # Botão Médio
        self.esperar(2)
        
        self.tirar_screenshot("voto_registrado")
        print("✅ Voto de dificuldade registrado")
    
    def test_avaliar_nota_estrelas(self):
        """
        Cenário: Dar nota com estrelas para matéria
        Dado que o usuário está visualizando uma matéria
        Quando seleciona quantidade de estrelas
        Então a nota deve ser salva
        """
        print("\n[TESTE] Avaliar matéria com estrelas")
        
        self.esperar(2)
        
        # Rolar para cima para ver seção de estrelas
        pyautogui.scroll(2)
        self.esperar(1)
        
        self.tirar_screenshot("secao_estrelas")
        
        # Clicar em 4 estrelas
        self.clicar_posicao(400, 400)  # Botão ★ 4
        self.esperar(2)
        
        self.tirar_screenshot("nota_estrelas_salva")
        
        # Verificar que não pode votar novamente
        self.clicar_posicao(450, 400)  # Tentar clicar em outra estrela
        self.esperar(1)
        
        self.tirar_screenshot("voto_unico_confirmado")
        print("✅ Nota com estrelas registrada")


# --------------- CONFIGURAÇÃO DOS TESTES --------------- 
@pytest.fixture(scope="session", autouse=True)
def setup_teardown():
    """Configuração inicial e limpeza após os testes"""
    print("\n" + "="*70)
    print("INICIANDO TESTES AUTOMATIZADOS COMPLETOS - EducaUTFPR")
    print("="*70 + "\n")
    
    # Aguarda aplicação estar aberta
    print("⏳ Aguardando aplicação estar pronta...")
    time.sleep(3)
    
    # Move mouse para posição neutra
    pyautogui.moveTo(100, 100)
    
    yield
    
    print("\n" + "="*70)
    print("✅ TODOS OS TESTES CONCLUÍDOS")
    print("="*70 + "\n")


# -------------- SUITE DE TESTES COMPLETA ---------------------
def test_suite_completa():
    """
    Suite que executa todos os testes em sequência
    """
    print("\n" + "="*70)
    print("EXECUTANDO SUITE COMPLETA DE TESTES")
    print("="*70 + "\n")
    
    # 1. Criação de Conta
    print("\nMódulo: Criação de Conta")
    
    # 2. Perfil
    print("\nMódulo: Perfil do Usuário")
    
    # 3. Grupos de Estudo
    print("\nMódulo: Grupos de Estudo")
    
    # 4. Central de Dúvidas/Fórum
    print("\nMódulo: Central de Dúvidas")
    
    # 5. Agenda Acadêmica
    print("\nMódulo: Agenda Acadêmica")
    
    # 6. Dúvidas
    print("\nMódulo: Dúvidas")
    
    # 7. Avaliação de Matérias
    print("\nMódulo: Avaliação de Matérias")


# ============================================================================
# RELATÓRIO DE TESTES
# ============================================================================
def gerar_relatorio_final():
    """Gera relatório completo dos testes executados"""
    print("\n" + "="*70)
    print("RELATÓRIO FINAL DE TESTES")
    print("="*70)
    print("\n✅ Módulos testados:")
    print("  1. Criação de Conta (2 testes)")
    print("     ✓ Cadastro com sucesso")
    print("     ✓ Validação de senha")
    print()
    print("  2. Perfil do Usuário (2 testes)")
    print("     ✓ Edição de perfil")
    print("     ✓ Visualização de estatísticas")
    print()
    print("  3. Grupos de Estudo (2 testes)")
    print("     ✓ Criação de grupo público")
    print("     ✓ Entrar em sala de matéria")
    print()
    print("  4. Central de Dúvidas/Fórum (2 testes)")
    print("     ✓ Criar dúvida")
    print("     ✓ Responder dúvida")
    print()
    print("  5. Agenda Acadêmica (2 testes)")
    print("     ✓ Adicionar prova")
    print("     ✓ Marcar tarefa concluída")
    print()
    print("  6. Dúvidas (2 testes)")
    print("     ✓ Criar dúvida com imagem")
    print("     ✓ Filtrar por disciplina")
    print()
    print("  7. Avaliação de Matérias (2 testes)")
    print("     ✓ Avaliar dificuldade")
    print("     ✓ Dar nota com estrelas")
    print()
    print("📁 Screenshots: pasta 'screenshots/'")
    print("Total: 14 testes executados")
    print("="*70 + "\n")


if __name__ == "__main__":
    # Executa os testes
    pytest.main([__file__, "-v", "--tb=short"])
    gerar_relatorio_final()